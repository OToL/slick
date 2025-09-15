--%-GIn\ file\ include\ %.%#
vim.cmd([[
    set errorformat=%f:%l:%c:\ %t%*[^:]:\ %m
]])

local function file_exists(path)
  local f = io.open(path, "r")
  if f then f:close() return true end
  return false
end

local function copy_file(src, dst)
  local _req, err = vim.uv.fs_copyfile(src, dst)
  if err then
    return nil, err
  end
  return true
end

local function rename_file(src, dst)
  if file_exists(dst) then
    os.remove(dst)  -- delete existing destination
  end

  local ok, err = os.rename(src, dst)
  if not ok then
    vim.notify("Rename failed: " .. (err or "unknown error"), vim.log.levels.ERROR)
    return false
  end

  return true
end

local function get_mtime(path)
  local stat = vim.uv.fs_stat(path)
  return stat and stat.mtime.sec or nil
end

local function get_workspace_root_directory_path()
    -- local current_file_path = debug.getinfo(1, "S").source:sub(2)
    -- local current_dir = vim.fn.fnamemodify(current_file_path, ':h')
    local ws = require('workspaces')
    return ws.path()
end

local function get_build_directory_path()
    return vim.fs.joinpath(get_workspace_root_directory_path(), "_build")
end

local function escape_pattern_case_insensitive(str)
  -- Escape pattern characters first
  str = str:gsub("([^%w])", "%%%1")
  -- Replace each letter with [Aa] style pattern
  str = str:gsub("%a", function(letter)
    return string.format("[%s%s]", letter:lower(), letter:upper())
  end)
  return str
end

local function patch_ps3_commands()
    local ps3_sdk_root = os.getenv("PSL1GHT")
    if not ps3_sdk_root then
        vim.notify("Cannot find environment varianle 'PSL1GHT'", vim.log.levels.ERROR)
        return
    end

    local build_dir = get_build_directory_path()
    local ps3_build_dir = vim.fs.joinpath(build_dir, "ps3-debug")
    local backup_commands_file_path = vim.fs.joinpath(ps3_build_dir, "compile_commands.json.bak")
    local commands_file_path = vim.fs.joinpath(ps3_build_dir, "compile_commands.json")

    if not file_exists(commands_file_path) then
        vim.notify("Cannot locate ps3 compilation database: " .. commands_file_path, vim.log.levels.ERROR)
        return
    end

    if file_exists(backup_commands_file_path) and file_exists(commands_file_path) then
        local backup_commands_file_mtime = get_mtime(backup_commands_file_path)
        local commands_file_mtime = get_mtime(commands_file_path)

        -- file is already processed
        if commands_file_mtime > backup_commands_file_mtime then
            return
        end
    end

    -- patch the command database
    local replacements = {
      [vim.fs.joinpath(ps3_sdk_root, "ppu", "bin", "ppu-gcc")] = "/usr/local/bin/clang",
      [vim.fs.joinpath(ps3_sdk_root, "ppu", "bin", "ppu-g++")] = "/usr/local/bin/clang++",
      ["-mcpu=cell"] = "",
      ["-fmodulo-sched"] = "",
      ["-mhard-float"] = "",
    }

    -- read file
    local f = assert(io.open(commands_file_path, "r"))
    local content = f:read("*a")
    f:close()

    -- perform replacements
    for old, new in pairs(replacements) do
        -- gsub replaces all occurrences
        content = content:gsub(escape_pattern_case_insensitive(old), new)
    end

    rename_file(commands_file_path, backup_commands_file_path)

    -- write file back
    f = assert(io.open(commands_file_path, "w"))
    f:write(content)
    f:close()

end

-- Build Commands
-- TODO: support target config as optional paramter e.g. release, debug, etc.

vim.api.nvim_create_user_command('BuildPs3', function()
    local build_dir_path = get_build_directory_path()
    vim.o.makeprg = "ninja -C " .. vim.fs.joinpath(build_dir_path, "ps3-debug")
    vim.cmd('Build')
end, {})

vim.api.nvim_create_user_command('BuildMacos', function()
    local build_dir_path = get_build_directory_path()
    vim.o.makeprg = "ninja -C " .. vim.fs.joinpath(build_dir_path, "macos-debug")
    vim.cmd('Build')
end, {})

vim.api.nvim_create_user_command('LSPPs3', function()
    local workspace_root = get_workspace_root_directory_path()
    local clangd_path = vim.fs.joinpath(workspace_root, ".clangd")
    local clangd_ps3_path = vim.fs.joinpath(workspace_root, ".clangd_ps3")

    patch_ps3_commands()
    copy_file(clangd_ps3_path, clangd_path)
    vim.cmd('LspRestart')
end, {})

vim.api.nvim_create_user_command('LSPMacos', function()
    local workspace_root = get_workspace_root_directory_path()
    local clangd_path = vim.fs.joinpath(workspace_root, ".clangd")
    local clangd_macos_path = vim.fs.joinpath(workspace_root, ".clangd_macos")

    copy_file(clangd_macos_path, clangd_path)
    vim.cmd('LspRestart')
end, {})
