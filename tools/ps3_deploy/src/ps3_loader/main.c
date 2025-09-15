#include <dirent.h>
#include <fcntl.h>
#include <malloc.h>
#include <ppu-types.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <net/net.h>
#include <net/socket.h>
#include <netinet/in.h>

#include <sys/process.h>
#include <sys/thread.h>
#include <sysmodule/sysmodule.h>
#include <sysutil/sysutil.h>

#include <zip.h>
#include <zlib/zlib.h>

#include "pad.h"
#include "rsxutil.h"

#define MAX_BUFFERS 2

#define PS3LOADVER "v0.3"

#define PORT 4299
#define MAX_ARG_CNT 256

#define STACKSIZE (256 * 1024)

#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define ZIP_PATH "/dev_hdd0/tmp/ps3load"
#define SELF_PATH ZIP_PATH ".self"

#define ERROR(err, msg)                                                        \
  do {                                                                         \
    if ((err) < 0) {                                                           \
      fprintf(stderr, "PS3Load: " msg "\n");                                   \
      return 1;                                                                \
    }                                                                          \
  } while (0)

static vu32 exit_req = 0;
static vu32 running = 0;
static sys_ppu_thread_t thr0;
// static sys_ppu_thread_t thr1;

static vu32 exit_flag = 0;
static volatile int listen_socket = -1;
static gcmContextData *context;

void drawFrame(rsxBuffer *buffer, long frame) {
  s32 i, j;
  for (i = 0; i < buffer->height; i++) {
    s32 color = (i / (buffer->height * 1.0) * 256);
    // This should make a nice black to green graident
    color = (color << 8) | ((frame % 255) << 16);
    for (j = 0; j < buffer->width; j++)
      buffer->ptr[i * buffer->width + j] = color;
  }
}

static void log_file(char *data, ...) {
  static int s_idx = 0;

  va_list argptr;
  va_start(argptr, data);

  char log_path[255];
  strcpy(log_path, ZIP_PATH);

  char file_name[128];
  sprintf(file_name, "/log_%d.txt", s_idx);
  strcat(log_path, file_name);

  char msg_buffer[256];
  vsprintf(msg_buffer, data, argptr);

  int f = open(log_path, (O_CREAT | O_TRUNC | O_WRONLY),
               (S_IRWXU | S_IRWXG | S_IRWXO));
  write(f, msg_buffer, strlen(msg_buffer));
  close(f);

  va_end(argptr);

  ++s_idx;
}
static void program_exit_callback() {
  sysUtilUnregisterCallback(SYSUTIL_EVENT_SLOT0);
  gcmSetWaitFlip(context);
  rsxFinish(context, 1);
}

static void sysutil_exit_callback(u64 status, u64 param, void *usrdata) {
  switch (status) {
  case SYSUTIL_EXIT_GAME:
    exit_req = 1;
    break;
  case SYSUTIL_DRAW_BEGIN:
  case SYSUTIL_DRAW_END:
    break;
  default:
    break;
  }
}

static void empty_directory(const char *path) {
  DIR *dirp;
  struct dirent *entry;

  dirp = opendir(path);
  if (!dirp)
    return;

  while ((entry = readdir(dirp)) != NULL) {
    char newpath[0x440];

    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
      continue;

    strcpy(newpath, path);
    strcat(newpath, "/");
    strcat(newpath, entry->d_name);

    if (entry->d_type == DT_DIR) {
      empty_directory(newpath);
      rmdir(newpath);
    } else
      remove(newpath);
  }
  closedir(dirp);
}

void finalize() {
  u64 retval;
  log_file("join thread");
  sysThreadJoin(thr0, &retval);

  log_file("net deinit");
  netDeinitialize();

  log_file("unload modules");
  sysModuleUnload(SYSMODULE_PNGDEC);
  sysModuleUnload(SYSMODULE_JPGDEC);
  sysModuleUnload(SYSMODULE_FS);

  log_file("done finalizing");
}

void *host_addr = NULL;
rsxBuffer buffers[MAX_BUFFERS];
int currentBuffer = 0;

static void control_thread2(void *arg) {
  long frame = 0; // To keep track of how many frames we have rendered.
  padInfo padinfo;
  padData paddata;
  int i;
  // bool continue_process = true;

  flip(context, MAX_BUFFERS - 1);

  // Ok, everything is setup. Now for the main loop.
  while (1) {
    // Check the pads.
    ioPadGetInfo(&padinfo);
    for (i = 0; i < MAX_PADS; i++) {
      if (padinfo.status[i]) {
        ioPadGetData(i, &paddata);

        if (paddata.BTN_START || exit_req) {
          goto end;
        }
      }
    }

    waitFlip(); // Wait for the last flip to finish, so we can draw to the old
                // buffer
    drawFrame(&buffers[currentBuffer],
              frame++);                       // Draw into the unused buffer
    flip(context, buffers[currentBuffer].id); // Flip buffer onto screen

    currentBuffer++;
    if (currentBuffer >= MAX_BUFFERS)
      currentBuffer = 0;
  }

end:
  log_file("exit control thread");
  // gcmSetWaitFlip(context);
}

int initialize() {
  struct sockaddr_in server;
  int ret, i;
  u16 width;
  u16 height;

  sysModuleLoad(SYSMODULE_FS);
  sysModuleLoad(SYSMODULE_JPGDEC);
  sysModuleLoad(SYSMODULE_PNGDEC);

  ioPadInit(7);

  atexit(program_exit_callback);
  sysUtilRegisterCallback(0, sysutil_exit_callback, NULL);

  ret = netInitialize();
  ERROR(ret, "Error initializing network");

  host_addr = memalign(1024 * 1024, HOST_SIZE);
  context = initScreen(host_addr, HOST_SIZE);

  getResolution(&width, &height);
  for (i = 0; i < MAX_BUFFERS; i++)
    makeBuffer(&buffers[i], width, height, i);

  // clean old data
  mkdir(ZIP_PATH, 0777);
  empty_directory(ZIP_PATH);

  sysThreadCreate(&thr0, control_thread2, NULL, 999, STACKSIZE, THREAD_JOINABLE,
                  "PS3Load");

  memset(&server, 0, sizeof(struct sockaddr_in));
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = htonl(INADDR_ANY);
  server.sin_port = htons(PORT);

  listen_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  ERROR(listen_socket, "Error creating socket()");

  ret = bind(listen_socket, (struct sockaddr *)&server,
             sizeof(struct sockaddr_in));

  ERROR(ret, "failed to bind socket");

  ret = listen(listen_socket, 10);
  ERROR(ret, "Error calling listen()");

  return 0;
}

int terminate() {
  u64 retval;
  int i;

  if (listen_socket >= 0)
    shutdown(listen_socket, SHUT_RDWR);

  exit_req = 1;

  // cleanup
  sysThreadJoin(thr0, &retval);

  // gcmSetWaitFlip(context);
  for (i = 0; i < MAX_BUFFERS; i++)
    rsxFree(buffers[i].ptr);

  // rsxFinish(context, 1);
  free(host_addr);

  netDeinitialize();

  ioPadEnd();

  sysModuleUnload(SYSMODULE_PNGDEC);
  sysModuleUnload(SYSMODULE_JPGDEC);
  sysModuleUnload(SYSMODULE_FS);

  return 0;
}

struct MsgHeader
{
    uint32_t compressed_size;
    uint32_t uncomressed_size;
    uint16_t arg_cnt;
    uint8_t major_version;
    uint8_t minor_version;
};

int main(s32 argc, const char *argv[]) {
  int ret, i;

  initialize();

  // todo
  {
#define continueloop()                                                         \
  {                                                                            \
    close(f);                                                                  \
    goto reloop;                                                               \
  }

  reloop:
    while (!exit_flag) {
      printf("Waiting for connection...\n");
      int f = accept(listen_socket, NULL, NULL);
      printf("exit_flag: %02x\n", exit_flag);
      if (exit_flag) {
        log_file("exit flag set");
        break;
      }

      ERROR(f, "Error calling accept()");
      log_file(f >= 0 ? "accept succeeded" : "Error calling accept()");

      u32 magic = 0;
      if (read(f, &magic, sizeof(u32)) < 0) {
        log_file("wrong magic number size");
        continueloop();
      }

      if (magic != 0x48415858) {
        fprintf(stderr, "Wrong HAXX magic\n");
        log_file("wrong HAXX magic");
        continueloop();
      }

      log_file("good HAXX magic");

      struct MsgHeader header;
      if (read(f, &header, sizeof(header)) < 0) {
        log_file("wrong second magic size");
        continueloop();
      }

    uint32_t filesize = ntohl(header.compressed_size);
    uint32_t uncompressed = ntohl(header.uncomressed_size);
    uint16_t argslen = ntohs(header.arg_cnt);
    uint32_t test = 1;
    if (*(uint8_t*)&test == 0) {
        log_file("PS3 is big-endian\n");
    } else {
        log_file("PS3 is little-endian\n");
    }
  
#if 0
      if (read(f, &magic, sizeof(u32)) < 0) {
        log_file("wrong second magic size");
        continueloop();
      }

      log_file("Magic check done");

      u32 filesize = 0;
      u32 uncompressed = 0;
      u16 argslen = magic & 0xffff;
      if (read(f, &filesize, sizeof(u32)) < 0) {
        log_file("fail to read file size");
        continueloop();
      }

      log_file("compressed file size %d", filesize);

      if (read(f, &uncompressed, sizeof(u32)) < 0) {
        log_file("failed to read uncompressed file size");
        continueloop();
      }
#endif

      log_file("version %hu.%hu %d", header.major_version, header.minor_version, sizeof(header));
      log_file("compressed file size %u %u", filesize, header.compressed_size);
      log_file("uncompressed file size %d %d", uncompressed, header.uncomressed_size);
      log_file("arg len %d, %d", argslen, header.arg_cnt);

      printf("Receiving data....(%d/%d)\n", filesize, uncompressed);

      u32 cnt;
      u32 pos = 0;
      u8 *data = (u8 *)malloc(filesize);
      while (pos < filesize) {
        cnt = MIN(4096, filesize - pos);
        ret = read(f, data + pos, cnt);
        if (ret < 0) {
          log_file("failed to rcv file data");
          continueloop();
        }

        pos += ret;
      }

      log_file("File data received %d", pos);

      printf("Receiving arguments....(%d)\n", argslen);

      u8 *args = NULL;
      if (argslen) {
        args = (u8 *)malloc(argslen);
        if (read(f, args, argslen) < 0) {
          log_file("failed to receive args");
          continueloop();
        }
        log_file("args received");
      }

      close(f);

      printf("Decompressing...\n");

      log_file("Decompressing ..");

      if (uncompressed) {
        u8 *compressed = data;
        uLongf final = uncompressed;

        data = (u8 *)malloc(final);
        ret = uncompress(data, &final, compressed, filesize);
        if (ret != Z_OK)
          continue;

        free(compressed);

        if (uncompressed != final)
          continue;

        uncompressed = final;
      } else
        uncompressed = filesize;

      log_file("deflate done !");

      printf("Launching...\n");

      f = open(SELF_PATH, (O_CREAT | O_TRUNC | O_WRONLY),
               (S_IRWXU | S_IRWXG | S_IRWXO));
      ERROR(f, "Error opening temporary file");

      if (f < 0) {
        log_file("failed to dump elf file %d", f);
      } else {
        log_file("elf file created");
      }

      pos = 0;
      while (pos < uncompressed) {
        cnt = MIN(4096, uncompressed - pos);
        if (write(f, data + pos, cnt) != cnt) {
          log_file("failed to dump elf file");
          continueloop();
        }
        pos += cnt;
      }
      close(f);
      free(data);

      log_file("Elf file dumped: %s", SELF_PATH);

      char bootpath[MAXPATHLEN + 1];
      struct zip *archive = zip_open(SELF_PATH, ZIP_CHECKCONS, NULL);
      int files = zip_get_num_files(archive);

      log_file("zip file count %d", files);

      strcpy(bootpath, SELF_PATH);
      if (files > 0) {
        log_file("unzipping files");
        strcpy(bootpath, "");

        for (i = 0; i < files; i++) {
          char path[MAXPATHLEN + 1];
          const char *filename = zip_get_name(archive, i, 0);

          strcpy(path, ZIP_PATH);

          if (!filename)
            continue;
          if (filename[0] != '/')
            strcat(path, "/");

          strcat(path, filename);

#define ENDS_WITH(needle)                                                      \
  (strlen(filename) >= strlen(needle) &&                                       \
   !strcasecmp(filename + strlen(filename) - strlen(needle), needle))

          if (ENDS_WITH("EBOOT.BIN") || ENDS_WITH(".self"))
            strcpy(bootpath, path);

          if (filename[strlen(filename) - 1] != '/') {
            struct zip_stat st;
            struct zip_file *zfd;

            if (zip_stat_index(archive, i, 0, &st)) {
              fprintf(stderr, "Unable to access file \'%s\' in zip.\n",
                      filename);
              continue;
            }

            zfd = zip_fopen_index(archive, i, 0);
            if (!zfd) {
              fprintf(stderr, "Unable to open file \'%s\' in zip.\n", filename);
              continue;
            }

            f = open(path, (O_CREAT | O_TRUNC | O_WRONLY),
                     (S_IRWXU | S_IRWXG | S_IRWXO));
            ERROR(f, "Error opening temporary file");

            pos = 0;
            u8 *buffer = (u8 *)malloc(4096);
            while (pos < st.size) {
              cnt = MIN(4096, st.size - pos);
              if (zip_fread(zfd, buffer, cnt) != cnt)
                ERROR(1, "Error reading from zip");
              if (write(f, buffer, cnt) != cnt)
                continueloop();

              pos += cnt;
            }
            free(buffer);

            zip_fclose(zfd);
            close(f);
          } else
            mkdir(path, 0777);
        }
      }

      if (archive)
        zip_close(archive);
      if (!strlen(bootpath)) {
        log_file("no boot path ...");
        continue;
      }

      char *launchenvp[2];
      char *launchargv[MAX_ARG_CNT];

      memset(launchenvp, 0, sizeof(launchenvp));
      memset(launchargv, 0, sizeof(launchargv));

      launchenvp[0] = (char *)malloc(0x440);
      snprintf(launchenvp[0], 0x440, "ps3load=%s", argv[0]);

      log_file("preparing launch (%d args)", argslen);

      i = 0;
      pos = 0;
      while (pos < argslen) {
        int len = strlen((char *)(args + pos));
        if (!len)
          break;

        launchargv[i] = (char *)malloc(len + 1);
        strcpy(launchargv[i], (char *)(args + pos));

        log_file("add arg: %s", launchargv[i]);

        pos += (len + 1);
        i++;
      }


      char params[512] = {0};

      for (int j = 0; j != i; j++) {
        strcat(params, launchargv[j]);
        strcat(params, " ");
      }

      strcat(params, launchenvp[0]);

      log_file("????");
      log_file("launching %s %s!", bootpath, params);

      terminate();

      log_file("spawning !!!!!");

      sysProcessExitSpawn2(bootpath, (const char **)launchargv,
                           (const char **)launchenvp, NULL, 0, 1001,
                           SYS_PROCESS_SPAWN_STACK_SIZE_1M);

      log_file("woot");
    }
  }

  terminate();

  log_file("clean exit");

  return 0;
}

