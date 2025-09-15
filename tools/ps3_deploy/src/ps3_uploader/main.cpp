#include <sys/fcntl.h>
#include <sys/stat.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <libgen.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <print>
#include <cassert>

#include <zlib/zlib.h>

#include <core/base_types.hpp>

#include "../common.h"

static const char * desc_export = "export";

constexpr slk::u32 O_BINARY = 0;

#define MAX_ARGS_LEN 1024

static const char * envvar = "PS3LOAD";

static void print_errno(char const * operation_str)
{
    std::println(stderr, "{}: {}", operation_str, std::strerror(errno));
}

static bool tcp_write(slk::i32 s, const void * buf, slk::u32 len)
{
    slk::u32 left = len;
    slk::u32 block = 0;
    auto const * data = static_cast<slk::u8 const *>(buf);

    while (left)
    {
        block = send(s, data, left, 0);
        if (block < 0)
        {
            print_errno("error sending data via socket");
            return false;
        }

        if (block > left)
        {
            std::print(stderr, "invalid block size when sending data");
            return false;
        }

        left -= block;
        data += block;
    }

    return true;
}

static bool send_tcp(const char * host, const slk::u8 * buf, slk::u32 len, slk::u32 len_un, const char * args, slk::u16 args_len)
{
    sockaddr_in sa{};
    sa.sin_addr.s_addr = inet_addr(host);

    if (sa.sin_addr.s_addr == INADDR_NONE)
    {
        std::println("resolving {} ...", host);
        hostent * he = gethostbyname(host);

        if (!he)
        {
            std::println(stderr, "error resolving hostname");
            return false;
        }

        if (he->h_addrtype != AF_INET)
        {
            std::println(stderr, "unsupported address");
            return false;
        }

        sa.sin_addr.s_addr = *(reinterpret_cast<slk::u32 *>(he->h_addr));
    }

    slk::i32 const s = socket(PF_INET, SOCK_STREAM, 0);

    if (s < 0)
    {
        print_errno("error creating socket");
        return false;
    }

    sa.sin_port = htons(LD_TCP_PORT);
    sa.sin_family = AF_INET;

    std::println("connecting to {}:{}", inet_ntoa(sa.sin_addr), LD_TCP_PORT);

    // TODO: connection with timeout
    if (connect(s, reinterpret_cast<sockaddr *>(&sa), sizeof(sa)) == -1)
    {
        print_errno("error connecting");
        close(s);
        return false;
    }

    std::println("sending upload request");

    constexpr slk::u8 tcp_header[] = {'H', 'A', 'X', 'X'};
    if (!tcp_write(s, std::data(tcp_header), sizeof(tcp_header)))
    {
        print_errno("failed to send tcp header");
        close(s);
        return false;
    }

    static_assert(sizeof(MsgHeader) == 12, "invalid header size");

    std::println("sending message header ...");
    std::println("\t- version: {}.{}", PS3LOAD_VERSION_MAYOR, PS3LOAD_VERSION_MINOR);
    std::println("\t- arg count: {}", args_len);
    std::println("\t- uncompressed size: {}", len_un);
    std::println("\t- compressed size: {}",len);

    MsgHeader header = {.compressed_size = htonl(len),
                        .uncomressed_size = htonl(len_un),
                        .arg_cnt = htons(args_len),
                        .major_version = PS3LOAD_VERSION_MAYOR,
                        .minor_version = PS3LOAD_VERSION_MINOR};

    std::println("sending message header #2 ...");
    std::println("\t- version: {}.{}", header.major_version, header.minor_version);
    std::println("\t- arg count: {}", header.arg_cnt);
    std::println("\t- uncompressed size: {}", header.uncomressed_size);
    std::println("\t- compressed size: {}",header.compressed_size);

    if (!tcp_write(s, &header, sizeof(header)))
    {
        print_errno("failed to send message header");
        close(s);
        return false;
    }

    constexpr char const * base_send_msg = "sending data ";
    std::print(base_send_msg);

    slk::u32 len_left = len;
    slk::u8 const * curr_buf = buf;
    slk::u8 progress_cnt = 0;
    while (len_left)
    {
        slk::u32 const block = std::max(4 * 1024U, len_left);
        len_left -= block;

        if (!tcp_write(s, curr_buf, block))
        {
            print_errno("\r\033[2Kfailed to send data block");
            close(s);
            return false;
        }

        curr_buf += block;
        progress_cnt++;

        if (progress_cnt % 5)
        {
            std::print(".");
        }
        else
        {
            progress_cnt = 0;
            // \033[2K clears the whole line
            std::print("\r\033[2K{}", base_send_msg);
        }
        std::fflush(stdout);
    }
    std::println("\r\033[2Kdata sent successfully!");

    if (args_len)
    {
        std::println("sending arguments ({} bytes)", args_len);

        if (!tcp_write(s, args, args_len))
        {
            print_errno("failed to send arguments");
            close(s);
            return false;
        }
    }

    close(s);

    return true;
}

static void print_usage(const char * argv0)
{
    std::print(stderr,
               "tool usage error.\n"
               "example:\n"
               "\t{} <elf file path> <arguments>\n\n",
               argv0);
    exit(EXIT_FAILURE);
}

int main(int argc, char ** argv)
{
    std::println("ps3_loader_client v{}.{}", PS3LOAD_VERSION_MAYOR, PS3LOAD_VERSION_MINOR);
    std::println("original implemention from dhewg, #wiidev efnet\n");

    if (argc < 2)
    {
        print_usage(*argv);
    }

    char const * const ev = std::getenv(envvar);
    if (!ev)
    {
        std::print(stderr,
                   "set the environment variable {} to a valid destination.\n"
                   "examples:\n"
                   "\t{} {}=tcp:wii\n"
                   "\t{} {}=tcp:192.168.0.30\n\n",
                   envvar, desc_export, envvar, desc_export, envvar);

        return -1;
    }

    // TODO: use c++ filesystem
    slk::i32 const fd = open(argv[1], O_RDONLY | O_BINARY);
    if (fd < 0)
    {
        // TODO: add file name
        print_errno("error opening the file");
        exit(EXIT_FAILURE);
    }

    // TODO: use c++ filesystem
    struct stat st = {};
    if (fstat(fd, &st))
    {
        close(fd);
        perror("error stat'ing the file");
        exit(EXIT_FAILURE);
    }

    off_t const fsize = st.st_size;
    if (fsize < 512)
    {
        close(fd);
        std::println(stderr, "error: invalid file size");
        exit(EXIT_FAILURE);
    }

    void * buf = malloc(fsize);
    if (!buf)
    {
        close(fd);
        std::println(stderr, "out of memory");
        exit(EXIT_FAILURE);
    }

    if (read(fd, buf, fsize) != fsize)
    {
        close(fd);
        free(buf);
        perror("error reading the file");
        exit(EXIT_FAILURE);
    }
    close(fd);

    int res = 0;
    off_t len = fsize;
    slk::u32 len_un = 0;
    bool compress = true;

    constexpr slk::u8 zip_4cc[] = {0x50, 0x4B, 0x03, 0x04}; 
    // ZIP signature 50 4B 03 04 i.e. do not double compress
    //    > PK\x03\x04 
    if (!memcmp(buf, std::data(zip_4cc), sizeof(zip_4cc)))
        compress = false;

    if (compress)
    {
        auto bufzlen = (uLongf)((float)fsize * 1.02);
        void * bufz = malloc(bufzlen);
        if (!bufz)
        {
            std::println(stderr, "out of memory");
            exit(EXIT_FAILURE);
        }

        std::println("compressing {} bytes...", (slk::u32)fsize);
        fflush(stdout);

        res = compress2((Bytef *)bufz, &bufzlen, (const Bytef *)buf, (uLong)fsize, 6);
        if (res != Z_OK)
        {
            free(buf);
            free(bufz);
            std::println(stderr, "error compressing data: {}", res);
            exit(EXIT_FAILURE);
        }

        if (bufzlen < (slk::u32)fsize)
        {
            std::println("  > compressed from {} bytes to {} byets ({:.2f}%)", fsize, bufzlen, 100.0f * (float)bufzlen / (float)fsize);

            len = bufzlen;
            len_un = fsize;
            free(buf);
            buf = bufz;
        }
        else
        {
            std::println(" compressed size gained size, discarding");
            free(bufz);
        }
    }

    char args[MAX_ARGS_LEN];
    char * arg_pos = nullptr;
    slk::u16 args_len = 0, args_left = 0;

    arg_pos = args;
    args_left = MAX_ARGS_LEN;

    auto c = snprintf(arg_pos, args_left, "%s", basename(argv[1]));
    arg_pos += c + 1;
    args_left -= c + 1;

    if (argc > 2)
    {
        for (int i = 2; i < argc; ++i)
        {
            c = snprintf(arg_pos, args_left, "%s", argv[i]);

            if (c >= args_left)
            {
                free(buf);
                std::println(stderr, "argument string too long");
                exit(EXIT_FAILURE);
            }

            arg_pos += c + 1;
            args_left -= c + 1;
        }

        if (args_left < 1)
        {
            free(buf);
            std::println(stderr, "argument string too long");
            exit(EXIT_FAILURE);
        }
    }

    arg_pos[0] = 0;
    args_len = MAX_ARGS_LEN - args_left + 1;

    if (strncmp(ev, "tcp:", 4))
    {
        print_usage(*argv);
    }
    else
    {
        if (strlen(ev) < 5)
            print_usage(*argv);

        // (const char * host, const slk::u8 * buf, slk::u32 len, slk::u32 len_un, const char * args, slk::u16 args_len)
        res = send_tcp(&ev[4], (slk::u8 const *)buf, len, len_un, args, args_len);
    }

    if (res)
        std::println("done.");
    else
        std::println("transfer failed.");

    free(buf);

    return 0;
}
