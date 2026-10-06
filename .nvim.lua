--------------------------------------------------------------------------------------------------------
--                                         Utilities                                                  --
--------------------------------------------------------------------------------------------------------

local utils = require("utils")

local function expand_launch_vars(s)
  if type(s) ~= 'string' then return s end
  return (s:gsub('%${workspaceFolder}', vim.fn.getcwd())
           :gsub('%${env:([%w_]+)}', function(v) return os.getenv(v) or '' end))
end

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

local function get_build_dir_path()
    return vim.fs.joinpath(vim.fn.getcwd(), "_build")
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

local function reload_launch_projects()
    vim.g.projects = {}

    local projects = {}
    local dap = require('dap.ext.vscode')
    for _, c in ipairs(dap.getconfigs()) do
        local program = expand_launch_vars(c.program)
        if not vim.startswith(program, '/') then
            program = vim.fs.normalize(vim.fs.joinpath(vim.fn.getcwd(), program))
        end

        local project = {
            program = program,
            cwd = expand_launch_vars(c.cwd),
            args = {}
        }
        for _, arg in ipairs(c.args) do
            project.args[#project.args + 1] = expand_launch_vars(arg)
        end

        projects[c.name] = project
    end
    vim.g.projects = projects
end

--------------------------------------------------------------------------------------------------------
--                                  Globl workspace variables                                         --
--------------------------------------------------------------------------------------------------------

vim.g.active_project = vim.g.active_project or "default"
vim.g.active_platform = vim.g.active_platform or get_current_platform()
vim.g.projects = {}

--------------------------------------------------------------------------------------------------------
--                                      Quick app launch                                              --
--------------------------------------------------------------------------------------------------------

local WEZTERM = get_wezterm_path()
if not WEZTERM then
    vim.notify("WEZTERM_EXECUTABLE env var is not defined", vim.log.levels.ERROR)
    return
end

-- Launch active program using the workspace root as current directory
function LaunchActiveProject(show_term)
    if not vim.g.active_project then
        vim.notify("No active project to launch", vim.log.levels.ERROR)
    end

    local project_desc = vim.g.projects[vim.g.active_project]
    if not project_desc then
        vim.notify("Cannot find project: " .. vim.g.active_project, vim.log.levels.ERROR)
    end

    local working_dir = vim.fn.getcwd()
    if project_desc.cwd then
       working_dir = project_desc.cwd
    end

    -- TODO: args
    if show_term then
        local height = math.floor(vim.api.nvim_win_get_height(0) / 4)
        vim.cmd('belowright ' .. height .. 'new')
        vim.fn.jobstart(project_desc.program, { cwd = working_dir, term = true })
        vim.cmd('wincmd p')
    else
        print(project_desc.program)
        print(working_dir)
        vim.fn.jobstart(project_desc.program, { cwd = working_dir, detach = true })
    end
end

-- User command to set active program
-- With an argument: set it directly (e.g. for use in startup config).
-- With no argument: open an interactive picker, as before.
vim.api.nvim_create_user_command('SetActiveProject', function(opts)
    reload_launch_projects()

    if opts.args ~= '' then
        if not vim.g.projects[opts.args] then
            vim.notify("Unknown project: " .. opts.args, vim.log.levels.ERROR)
            return
        end

        vim.g.active_project = opts.args
        vim.notify("Active project set to: " .. opts.args)
        return
    end

    local pickers      = require("telescope.pickers")
    local finders      = require("telescope.finders")
    local conf         = require("telescope.config").values
    local actions      = require("telescope.actions")
    local action_state = require("telescope.actions.state")

    local projects     = vim.tbl_keys(vim.g.projects)
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
end, {
    nargs = '?',
    complete = function()
        return vim.tbl_keys(vim.g.projects)
    end,
})

vim.api.nvim_create_user_command('ActiveProject', function(opts)
    vim.notify("Active Project: " .. vim.g.active_project, vim.log.levels.INFO)

    -- Force Noice plugin update because otherwise notification is shown next time cmd mode is entered
    local ok, router = pcall(require, "noice.message.router")
    if ok and router.update then
        require("noice.message.router").update()
    end
end, {})

--%-GIn\ file\ include\ %.%#
vim.cmd([[
    set errorformat=%f:%l:%c:\ %t%*[^:]:\ %m
]])

--------------------------------------------------------------------------------------------------------
--                                      BUILD COMMANDS                                                --
--------------------------------------------------------------------------------------------------------

vim.api.nvim_create_user_command('SetActiveTargetPlatform', function(opts)
    local platform = (opts.fargs[1] or ''):lower()
    local config   = (opts.fargs[2] or 'debug'):lower()
    if platform == '' then
        vim.notify('Usage: :SetActiveTargetPlatform <Darwin|Windows|Linux> [config]', vim.log.levels.ERROR)
        return
    end

    local preset    = platform .. '-' .. config
    local build_dir = vim.fs.joinpath(get_build_dir_path(), preset)
    if vim.fn.isdirectory(build_dir) == 0 then
        vim.notify('Build directory not found: ' .. build_dir, vim.log.levels.WARN)
    end

    vim.g.target_build_platform = platform
    vim.g.target_build_config   = config
    vim.g.target_build_dir      = build_dir
    vim.o.makeprg               = 'ninja -C ' .. build_dir

    vim.notify('Active target: ' .. preset, vim.log.levels.INFO)
end, {
    nargs = '+',
    complete = function(_, cmdline)
        local n = #vim.split(cmdline, '%s+')
        return n <= 2 and { 'Darwin', 'Windows', 'Linux' } or { 'debug', 'relwithdebinfo' }
    end,
})

vim.api.nvim_create_user_command('BuildCurrentFile', function()
    local file = vim.fn.expand('%:p')
    local build_dir = vim.g.target_build_dir
    if not build_dir then
        vim.notify('No active target platform; run :SetActiveTargetPlatform first', vim.log.levels.ERROR)
        return
    end

    local ccj = vim.fs.joinpath(build_dir, 'compile_commands.json')
    if vim.fn.filereadable(ccj) == 0 then
        vim.notify('compile_commands.json not found at ' .. ccj, vim.log.levels.ERROR)
        return
    end

    local db = vim.json.decode(table.concat(vim.fn.readfile(ccj), '\n'))
    for _, e in ipairs(db) do
        if vim.fn.fnamemodify(e.file, ':p') == file then
            -- ninja targets are relative to the build dir; strip it if `output` is absolute
            local output = e.output:gsub('^' .. vim.pesc(build_dir) .. '[/\\]', '')
            local saved = vim.o.makeprg
            vim.o.makeprg = 'ninja -C ' .. build_dir .. ' ' .. output
            vim.cmd('Build')
            vim.o.makeprg = saved
            return
        end
    end
    vim.notify('No compile command found for this file', vim.log.levels.WARN)
end, {})

vim.api.nvim_create_user_command('ReloadLaunchProjects', function()
    reload_launch_projects()
end, {})

--------------------------------------------------------------------------------------------------------
--                                       KEY BINDINGS                                                 --
--------------------------------------------------------------------------------------------------------

vim.api.nvim_set_keymap("n", "<F6>", '<cmd>lua LaunchActiveProject(false)<cr>', { noremap = true, silent = true })
utils.set_multi_keymap("n", { "<M-F6>", "<F54>" }, '<cmd>lua LaunchActiveProject(true)<cr>',
    { noremap = true, silent = true })

vim.api.nvim_set_keymap("n", "<F7>", '<cmd>Build<cr>', { noremap = true, silent = true })
utils.set_multi_keymap("n", { "<M-F7>", "<F55>" }, '<cmd>BuildCurrentFile<cr>', { noremap = true, silent = true })

--------------------------------------------------------------------------------------------------------
--                                      STARTUP DEFAULTS                                              --
--------------------------------------------------------------------------------------------------------

vim.cmd('SetActiveTargetPlatform Darwin debug')
vim.cmd('SetActiveProject slick demos')
