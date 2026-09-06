--------------------------------------------------------------------------------------------------------
--                                         Utilities                                                  --
--------------------------------------------------------------------------------------------------------

local utils = require("utils")

-- Detect current platform
local function get_current_platform()
  if vim.fn.has("macunix") == 1 then
    return "darwin"
  elseif vim.fn.has("win32") == 1 or vim.fn.has("win64") == 1 then
    return "windows"
  elseif vim.fn.has("unix") == 1 then
    return "linux"
  else
    return "unknown"
  end
end

function GetBuildDirPath()
    return vim.fs.joinpath(vim.fn.getcwd(), "_build")
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

local function get_wezterm_path()
    local wz_path = os.getenv("WEZTERM_EXECUTABLE")
    if not wz_path then
        return nil
    end
    local wz_dir = vim.fn.fnamemodify(wz_path, ":h")
    if vim.g.active_platform == "windows" then
        return utils.joinPaths(wz_dir, "wezterm.exe")
    else
       return utils.joinPaths(wz_dir, "wezterm")
    end
end

--------------------------------------------------------------------------------------------------------
--                                  Globl workspace variables                                         --
--------------------------------------------------------------------------------------------------------

vim.g.active_project = vim.g.active_project or "default"
vim.g.active_platform = vim.g.active_platform or get_current_platform()

--------------------------------------------------------------------------------------------------------
--                                      Quick app launch                                              --
--------------------------------------------------------------------------------------------------------

-- Project launch configurations
local function get_project_commands()
    local curr_platform = get_current_platform()
    local build_dir = GetBuildDirPath()
    return {
        ["cpp_sandbox"]     = vim.fs.joinpath(build_dir, curr_platform .. "-debug/samples/cpp_sandbox/cpp_sandbox"),
        ["graphics_sandbox"]= vim.fs.joinpath(build_dir, curr_platform .. "-debug/samples/graphics_sandbox/graphics_sandbox"),
        ["test_bgfx"]       = vim.fs.joinpath(build_dir, curr_platform .. "-debug/samples/test_bgfx/test_bgfx"),
        ["asteroids"]       = vim.fs.joinpath(build_dir, curr_platform .. "-debug/samples/asteroids/asteroids"),
    }
end

function GetLaunchCommand(project)
    local commands = get_project_commands()
    return commands[project] or "echo 'No project selected'"
end

local WEZTERM = get_wezterm_path()
if not WEZTERM then
    vim.notify("WEZTERM_EXECUTABLE env var is not defined", vim.log.levels.ERROR)
    return
end

-- Launch active program using the workspace root as current directory
function LaunchActiveProject(show_term)
    local command = GetLaunchCommand(vim.g.active_project)
    local workspace_dir = vim.fn.getcwd()

    if show_term then
        local height = math.floor(vim.api.nvim_win_get_height(0) / 4)
        vim.cmd('belowright ' .. height .. 'new')
        vim.fn.jobstart(command, { cwd = workspace_dir, term = true })
        vim.cmd('wincmd p')
    else
        vim.fn.jobstart(command, { cwd = workspace_dir, detach = true })
    end
end

-- User command to set active program
vim.api.nvim_create_user_command('SetActiveProject', function()
    local pickers     = require("telescope.pickers")
    local finders     = require("telescope.finders")
    local conf        = require("telescope.config").values
    local actions     = require("telescope.actions")
    local action_state = require("telescope.actions.state")

    local projects = vim.tbl_keys(get_project_commands())
    table.sort(projects)

    pickers.new({}, {
        prompt_title = "Set Active Project",
        finder = finders.new_table({ results = projects }),
        sorter = conf.generic_sorter({}),
        attach_mappings = function(prompt_bufnr)
            actions.select_default:replace(function()
                actions.close(prompt_bufnr)
                local selection = action_state.get_selected_entry()
                vim.g.active_project = selection[1]
                vim.notify("Active project set to: " .. selection[1])
            end)
            return true
        end,
    }):find()
end, { nargs = 0 })

vim.api.nvim_create_user_command('ActiveProject', function(opts)
    vim.notify("Active Project: " .. vim.g.active_project, vim.log.levels.INFO)

    -- Force Noice plugin update because otherwise notification is shown next time cmd mode is entered
    local ok, router = pcall(require, "noice.message.router")
    if ok and router.update then
        require("noice.message.router").update()
    end
end, { })

vim.api.nvim_set_keymap("n", "<F6>", '<cmd>lua LaunchActiveProject(false)<cr>', {noremap = true, silent = true})
vim.api.nvim_set_keymap("n", "<F7>", '<cmd>Build<cr>', {noremap = true, silent = true})

vim.api.nvim_set_keymap("n", "<M-F6>", '<cmd>lua LaunchActiveProject(true)<cr>', {noremap = true, silent = true})
vim.api.nvim_set_keymap("n", "<F54>", '<cmd>lua LaunchActiveProject(true)<cr>', {noremap = true, silent = true})


--%-GIn\ file\ include\ %.%#
vim.cmd([[
    set errorformat=%f:%l:%c:\ %t%*[^:]:\ %m
]])

--------------------------------------------------------------------------------------------------------
--                                      BUILD COMMANDS                                                --
--------------------------------------------------------------------------------------------------------

vim.api.nvim_create_user_command('BuildDarwin', function()
  local build_dir_path = GetBuildDirPath()
  vim.o.makeprg = "ninja -C " .. vim.fs.joinpath(build_dir_path, "darwin-debug")
    vim.cmd('Build')
  -- vim.cmd('silent! wall')  -- Save all, ignore errors
  -- vim.cmd('make')          -- Use make instead of Build
end, {})

