--------------------------------------------------------------------------------------------------------
--                                         Utilities                                                  --
--------------------------------------------------------------------------------------------------------

-- Detect current platform
local function get_current_platform()
  if vim.fn.has("macunix") == 1 then
    return "macos"
  elseif vim.fn.has("win32") == 1 or vim.fn.has("win64") == 1 then
    return "windows"
  elseif vim.fn.has("unix") == 1 then
    return "linux"
  else
    return "unknown"
  end
end

function GetWorkspaceRootDirPath()
    -- local current_file_path = debug.getinfo(1, "S").source:sub(2)
    -- local current_dir = vim.fn.fnamemodify(current_file_path, ':h')
    local ws = require('workspaces')
    return ws.path()
end

function GetBuildDirPath()
    return vim.fs.joinpath(GetWorkspaceRootDirPath(), "_build")
end

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

local function escape_pattern_case_insensitive(str)
  -- Escape pattern characters first
  str = str:gsub("([^%w])", "%%%1")
  -- Replace each letter with [Aa] style pattern
  str = str:gsub("%a", function(letter)
    return string.format("[%s%s]", letter:lower(), letter:upper())
  end)
  return str
end

--------------------------------------------------------------------------------------------------------
--                                  Globl workspace variables                                         --
--------------------------------------------------------------------------------------------------------

vim.g.active_project = vim.g.active_project or "default"
vim.g.active_platform = vim.g.active_platform or get_current_platform()

-- Load vscode launch description
require('dap.ext.vscode').load_launchjs("launch.json", { codelldb = { 'cpp' } })

--------------------------------------------------------------------------------------------------------
--                                      Quick app launch                                              --
--------------------------------------------------------------------------------------------------------

-- Project launch configurations
function GetLaunchCommand(project)
  local curr_platform = get_current_platform()
  local commands = {
    ["default"] = "echo 'No project selected'",
    ["ps3_upload"] = vim.fs.joinpath(GetBuildDirPath(), curr_platform .. "-debug/tools/ps3_deploy/src/ps3_uploader/ps3_uploader"),
    ["cpp_sandbox"] = vim.fs.joinpath(GetBuildDirPath(), curr_platform .. "-debug/samples/cpp_sandbox/cpp_sandbox"),
    ["graphics_sandbox"] = vim.fs.joinpath(GetBuildDirPath(), curr_platform .. "-debug/samples/graphics_sandbox/graphics_sandbox"),
  }
  return commands[project] or commands["default"]
end

-- Launch active program
function LaunchActiveProject()
  local command = GetLaunchCommand(vim.g.active_project)
  vim.cmd(string.format('!tmux send-keys -t 2 C-u "%s" Enter', command))
  vim.cmd('!tmux resize-pane')
end

-- User command to set active program
vim.api.nvim_create_user_command('SetActiveProject', function(opts)
  vim.g.active_project = opts.args
  vim.notify("Active program set to: " .. opts.args)
end, { nargs = 1 })

vim.api.nvim_set_keymap("n", "<M-F6>", '<cmd>lua LaunchActiveProject()<cr>', {noremap = true, silent = true})
vim.api.nvim_set_keymap("n", "<M-F7>", '<cmd>Build<cr>', {noremap = true, silent = true})

--%-GIn\ file\ include\ %.%#
vim.cmd([[
    set errorformat=%f:%l:%c:\ %t%*[^:]:\ %m
]])

--------------------------------------------------------------------------------------------------------
--                                  LSP platform switch                                               --
--------------------------------------------------------------------------------------------------------


local function patch_ps3_commands()
    local ps3_sdk_root = os.getenv("PSL1GHT")
    if not ps3_sdk_root then
        vim.notify("Cannot find environment varianle 'PSL1GHT'", vim.log.levels.ERROR)
        return
    end

    local build_dir = GetBuildDirPath()
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
    local build_dir_path = GetBuildDirPath()
    vim.o.makeprg = "ninja -C " .. vim.fs.joinpath(build_dir_path, "ps3-debug")
    vim.cmd('Build')
end, {})

vim.api.nvim_create_user_command('BuildMacos', function()
  local build_dir_path = GetBuildDirPath()
  vim.o.makeprg = "ninja -C " .. vim.fs.joinpath(build_dir_path, "macos-debug")
    vim.cmd('Build')
  -- vim.cmd('silent! wall')  -- Save all, ignore errors
  -- vim.cmd('make')          -- Use make instead of Build
end, {})

-- vim.api.nvim_create_user_command('BuildMacos', function()
--     local build_dir_path = GetBuildDirPath()
--     vim.o.makeprg = "ninja -C " .. vim.fs.joinpath(build_dir_path, "macos-debug")
--     print(vim.o.makeprg)
--     vim.cmd('Build')
-- end, {})

vim.api.nvim_create_user_command('LSPPs3', function()
    local workspace_root = GetWorkspaceRootDirPath()
    local clangd_path = vim.fs.joinpath(workspace_root, ".clangd")
    local clangd_ps3_path = vim.fs.joinpath(workspace_root, ".clangd_ps3")

    patch_ps3_commands()
    copy_file(clangd_ps3_path, clangd_path)
    vim.cmd('LspRestart')
end, {})

vim.api.nvim_create_user_command('LSPMacos', function()
    local workspace_root = GetWorkspaceRootDirPath()
    local clangd_path = vim.fs.joinpath(workspace_root, ".clangd")
    local clangd_macos_path = vim.fs.joinpath(workspace_root, ".clangd_macos")

    copy_file(clangd_macos_path, clangd_path)
    vim.cmd('LspRestart')
end, {})
