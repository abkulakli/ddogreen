@echo off
REM ddogreen Windows installer script
REM Installs and manages ddogreen Windows service
REM Similar to Linux TGZ packaging functionality

setlocal enabledelayedexpansion

REM Service configuration
set SERVICE_NAME=ddogreen
set SERVICE_DISPLAY_NAME=DDOGreen by DDOSoft - Intelligent Green Power Management for Sustainable Computing
set SERVICE_DESCRIPTION=Automatically manages Windows power plans based on system load monitoring for sustainable computing
set TARGET_EXECUTABLE_PATH=%ProgramFiles%\DDOSoft\ddogreen\ddogreen.exe
set CONFIG_DIR=%ProgramData%\DDOSoft\ddogreen
set CONFIG_FILE=%CONFIG_DIR%\ddogreen.conf
set LOG_FILE=%ProgramData%\DDOSoft\ddogreen\ddogreen.log

REM Colors for output (using ANSI escape codes if supported)
set "RED=[31m"
set "GREEN=[32m"
set "YELLOW=[33m"
set "BLUE=[34m"
set "NC=[0m"

REM Function to print colored output
goto :parse_args

:print_info
echo [INFO] %~1
goto :eof

:print_success
echo [SUCCESS] %~1
goto :eof

:print_warning
echo [WARNING] %~1
goto :eof

:print_error
echo [ERROR] %~1
goto :eof

REM Function to check if running as administrator
:check_admin
net session >nul 2>&1
if %errorlevel% neq 0 (
    call :print_error "This script must be run as Administrator"
    echo.
    echo Right-click on Command Prompt and select "Run as administrator"
    echo Then run this script again.
    exit /b 1
)
goto :eof

REM Function to check if PowerShell is available
:check_powershell
powershell -Command "Write-Host 'PowerShell check'" >nul 2>&1
if %errorlevel% neq 0 (
    call :print_error "PowerShell not found. This installer requires PowerShell."
    exit /b 1
)
goto :eof

REM Function to install ddogreen
:install_ddogreen
call :print_info "Installing ddogreen..."

REM Check if binary exists in current directory or bin/ subdirectory
set DDOGREEN_BINARY=
if exist "ddogreen.exe" (
    set DDOGREEN_BINARY=ddogreen.exe
) else if exist "bin\ddogreen.exe" (
    set DDOGREEN_BINARY=bin\ddogreen.exe
) else (
    call :print_error "ddogreen.exe not found in current directory or bin\ subdirectory"
    call :print_error "Please run this script from the extracted ZIP directory"
    exit /b 1
)

REM Check if service already exists and stop it
sc query "%SERVICE_NAME%" >nul 2>&1
if %errorlevel% equ 0 (
    call :print_warning "ddogreen service already exists"
    call :print_info "Stopping existing service..."
    sc stop "%SERVICE_NAME%" >nul 2>&1
    timeout /t 5 /nobreak >nul
    
    call :print_info "Removing existing service..."
    sc delete "%SERVICE_NAME%" >nul 2>&1
    timeout /t 2 /nobreak >nul
)

REM Create program directory
call :print_info "Creating program directory..."
if not exist "%ProgramFiles%\DDOSoft\ddogreen" (
    mkdir "%ProgramFiles%\DDOSoft\ddogreen"
    if %errorlevel% neq 0 (
        call :print_error "Failed to create program directory"
        exit /b 1
    )
)

REM Install configuration first
call :print_info "Setting up configuration..."
if not exist "%CONFIG_DIR%" (
    mkdir "%CONFIG_DIR%"
    if %errorlevel% neq 0 (
        call :print_error "Failed to create configuration directory"
        exit /b 1
    )
)

if not exist "%CONFIG_FILE%" (
    if exist "data\ddogreen.conf.default" (
        copy "data\ddogreen.conf.default" "%CONFIG_FILE%" >nul
        call :print_success "Default configuration installed at %CONFIG_FILE%"
    ) else if exist "ddogreen.conf.default" (
        copy "ddogreen.conf.default" "%CONFIG_FILE%" >nul
        call :print_success "Default configuration installed at %CONFIG_FILE%"
    ) else if exist "ddogreen.conf" (
        copy "ddogreen.conf" "%CONFIG_FILE%" >nul
        call :print_success "Default configuration installed at %CONFIG_FILE%"
    ) else (
        call :print_warning "Configuration template not found in package"
        call :print_warning "You may need to create %CONFIG_FILE% manually"
    )
) else (
    call :print_info "Existing configuration preserved at %CONFIG_FILE%"
)

REM Install the executable
call :print_info "Installing executable to %TARGET_EXECUTABLE_PATH%..."
copy "%DDOGREEN_BINARY%" "%TARGET_EXECUTABLE_PATH%" >nul
if %errorlevel% neq 0 (
    call :print_error "Failed to copy executable"
    exit /b 1
)

REM Create log file
if not exist "%LOG_FILE%" (
    type nul > "%LOG_FILE%"
)

REM Create and install Windows service
call :print_info "Creating Windows service..."
sc create "%SERVICE_NAME%" binPath= "\"%TARGET_EXECUTABLE_PATH%\" --daemon --config \"%CONFIG_FILE%\"" DisplayName= "%SERVICE_DISPLAY_NAME%" start= auto
if %errorlevel% neq 0 (
    call :print_error "Failed to create service"
    exit /b 1
)

REM Set service description
sc description "%SERVICE_NAME%" "%SERVICE_DESCRIPTION%"

REM Set service to restart on failure
sc failure "%SERVICE_NAME%" reset= 86400 actions= restart/10000/restart/30000/restart/60000

REM Start the service
call :print_info "Starting service..."
sc start "%SERVICE_NAME%" >nul
if %errorlevel% equ 0 (
    call :print_success "ddogreen installed and started successfully!"
    echo.
    call :print_info "Service Status:"
    sc query "%SERVICE_NAME%"
    echo.
    call :print_info "To view logs: Get-WinEvent -LogName Application -Source ddogreen"
    call :print_info "To stop:      sc stop ddogreen"
    call :print_info "To restart:   sc stop ddogreen && sc start ddogreen"
    echo.
    call :print_success "ddogreen is now managing your system's power automatically!"
) else (
    call :print_error "Service installation failed"
    call :print_info "Check Windows Event Log: Get-WinEvent -LogName Application -Source ddogreen"
    exit /b 1
)
goto :eof

REM Function to uninstall ddogreen
:uninstall_ddogreen
call :print_info "Uninstalling ddogreen..."

REM Stop and remove service
call :print_info "Stopping and removing service..."
sc stop "%SERVICE_NAME%" >nul 2>&1
timeout /t 5 /nobreak >nul
sc delete "%SERVICE_NAME%" >nul 2>&1

REM Remove executable and program directory
call :print_info "Removing executable..."
if exist "%TARGET_EXECUTABLE_PATH%" (
    del "%TARGET_EXECUTABLE_PATH%" >nul 2>&1
)
if exist "%ProgramFiles%\DDOSoft\ddogreen" (
    rmdir "%ProgramFiles%\DDOSoft\ddogreen" >nul 2>&1
)
if exist "%ProgramFiles%\DDOSoft" (
    rmdir "%ProgramFiles%\DDOSoft" >nul 2>&1
)

REM Clean up remaining files
call :print_info "Cleaning up files..."
if exist "%LOG_FILE%" (
    del "%LOG_FILE%" >nul 2>&1
)

REM Remove configuration directory (ask user)
if exist "%CONFIG_DIR%" (
    echo.
    set /p remove_config="Remove configuration directory %CONFIG_DIR%? [y/N]: "
    if /i "!remove_config!"=="y" (
        rmdir /s /q "%CONFIG_DIR%" >nul 2>&1
        call :print_info "Configuration directory removed."
    ) else (
        call :print_info "Configuration directory preserved."
    )
)

call :print_success "ddogreen has been uninstalled."
goto :eof

REM Function to show status
:show_status
call :print_info "ddogreen Status:"

if exist "%TARGET_EXECUTABLE_PATH%" (
    call :print_success "Executable: %TARGET_EXECUTABLE_PATH% (installed)"
) else (
    call :print_warning "Executable: Not installed"
)

sc query "%SERVICE_NAME%" >nul 2>&1
if %errorlevel% equ 0 (
    call :print_success "Service: %SERVICE_NAME% (installed)"
    
    sc query "%SERVICE_NAME%" | find "RUNNING" >nul
    if %errorlevel% equ 0 (
        call :print_success "Status: Running"
    ) else (
        call :print_warning "Status: Stopped"
    )
    
    REM Check if service is set to auto-start
    sc qc "%SERVICE_NAME%" | find "AUTO_START" >nul
    if %errorlevel% equ 0 (
        call :print_success "Auto-start: Enabled"
    ) else (
        call :print_warning "Auto-start: Disabled"
    )
) else (
    call :print_warning "Service: Not installed"
)
goto :eof

REM Function to show usage
:show_usage
echo ddogreen Windows Installer
echo.
echo Usage: installer.bat [OPTION]
echo.
echo Options:
echo   --install     Install ddogreen service
echo   --uninstall   Uninstall ddogreen service
echo   --status      Show installation status
echo   --help        Show this help message
echo.
echo Examples:
echo   installer.bat --install         # Install and start service
echo   installer.bat --uninstall       # Stop and remove service
echo   installer.bat --status          # Check current status
echo.
echo Requirements:
echo   - Windows 10/11 or Windows Server 2016+
echo   - Administrator privileges (run as Administrator)
echo   - PowerShell (pre-installed on modern Windows)
echo.
echo After installation:
echo   - Service starts automatically on boot
echo   - View logs: Get-WinEvent -LogName Application -Source ddogreen
echo   - Control service: sc {start^|stop^|query} ddogreen
echo.
goto :eof

REM Parse command line arguments
:parse_args
set ACTION=

REM Check if no arguments provided
if "%~1"=="" (
    call :print_error "No option specified"
    echo.
    call :show_usage
    exit /b 1
)

REM Parse first argument
if "%~1"=="--install" (
    set ACTION=install
) else if "%~1"=="--uninstall" (
    set ACTION=uninstall
) else if "%~1"=="--status" (
    set ACTION=status
) else if "%~1"=="--help" (
    set ACTION=help
) else (
    call :print_error "Unknown option: %~1"
    echo.
    call :show_usage
    exit /b 1
)

goto execute_action
REM Execute action
:execute_action
if "%ACTION%"=="install" (
    call :check_admin
    call :check_powershell
    call :install_ddogreen
) else if "%ACTION%"=="uninstall" (
    call :check_admin
    call :check_powershell
    call :uninstall_ddogreen
) else if "%ACTION%"=="status" (
    call :show_status
) else if "%ACTION%"=="help" (
    call :show_usage
) else (
    call :print_error "No option specified"
    echo.
    call :show_usage
    exit /b 1
)

endlocal