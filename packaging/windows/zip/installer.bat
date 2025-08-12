@echo off
REM DDOGreen Windows installer script
REM Installs and manages DDOGreen Windows service
REM Similar to Linux TGZ packaging functionality

setlocal enabledelayedexpansion

REM Service configuration
set SERVICE_NAME=ddogreen
set SERVICE_DISPLAY_NAME=DDOGreen by DDOSoft - Intelligent Green Power Management for Sustainable Computing
set SERVICE_DESCRIPTION=Automatically manages Windows power plans based on system load monitoring for sustainable computing
set TARGET_EXECUTABLE_PATH=%ProgramFiles%\ddosoft\ddogreen\ddogreen.exe
set CONFIG_DIR=%ProgramData%\ddosoft\ddogreen
set CONFIG_FILE=%CONFIG_DIR%\ddogreen.conf
set LOG_FILE=%ProgramData%\ddosoft\ddogreen\ddogreen.log

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

REM Function to install DDOGreen
:install_ddogreen
call :print_info "Installing DDOGreen..."

REM Check if binary exists in current directory or bin/ subdirectory
set DDOGREEN_BINARY=
if exist "ddogreen.exe" (
    set DDOGREEN_BINARY=ddogreen.exe
) else if exist "bin\ddogreen.exe" (
    set DDOGREEN_BINARY=bin\ddogreen.exe
) else (
    call :print_error "DDOGreen.exe not found in current directory or bin\ subdirectory"
    call :print_error "Please run this script from the extracted ZIP directory"
    exit /b 1
)

REM Check if service already exists and stop it
sc query "%SERVICE_NAME%" >nul 2>&1
if %errorlevel% equ 0 (
    call :print_warning "DDOGreen service already exists"
    call :print_info "Stopping existing service..."
    sc stop "%SERVICE_NAME%" >nul 2>&1
    timeout /t 5 /nobreak >nul

    call :print_info "Removing existing service..."
    sc delete "%SERVICE_NAME%" >nul 2>&1
    timeout /t 2 /nobreak >nul
)

REM Create program directory
call :print_info "Creating program directory..."
if not exist "%ProgramFiles%\ddosoft" (
    mkdir "%ProgramFiles%\ddosoft"
    if %errorlevel% neq 0 (
        call :print_error "Failed to create ddosoft directory"
        exit /b 1
    )
)
if not exist "%ProgramFiles%\ddosoft\ddogreen" (
    mkdir "%ProgramFiles%\ddosoft\ddogreen"
    if %errorlevel% neq 0 (
        call :print_error "Failed to create program directory"
        exit /b 1
    )
    call :print_success "Program directory created"
) else (
    call :print_info "Program directory already exists"
)

REM Verify directory is accessible
if not exist "%ProgramFiles%\ddosoft\ddogreen" (
    call :print_error "Program directory is not accessible: %ProgramFiles%\ddosoft\ddogreen"
    exit /b 1
)

REM Install configuration first
call :print_info "Setting up configuration..."
if not exist "%ProgramData%\ddosoft" (
    mkdir "%ProgramData%\ddosoft"
    if %errorlevel% neq 0 (
        call :print_error "Failed to create ddosoft data directory"
        exit /b 1
    )
)
if not exist "%CONFIG_DIR%" (
    mkdir "%CONFIG_DIR%"
    if %errorlevel% neq 0 (
        call :print_error "Failed to create configuration directory"
        exit /b 1
    )
    call :print_success "Configuration directory created"
) else (
    call :print_info "Configuration directory already exists"
)

REM Verify configuration directory is accessible
if not exist "%CONFIG_DIR%" (
    call :print_error "Configuration directory is not accessible: %CONFIG_DIR%"
    exit /b 1
)

if not exist "%CONFIG_FILE%" (
    if exist "share\ddogreen\ddogreen.conf.default" (
        copy "share\ddogreen\ddogreen.conf.default" "%CONFIG_FILE%" >nul
        call :print_success "Default configuration installed at %CONFIG_FILE%"
    ) else (
        call :print_error "Configuration template not found at share\ddogreen\ddogreen.conf.default"
        call :print_error "Package may be corrupted or incomplete"
        exit /b 1
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

REM Install NSSM service wrapper
set NSSM_PATH=%ProgramFiles%\ddosoft\ddogreen\nssm.exe
call :print_info "Installing NSSM service wrapper..."
copy "bin\nssm.exe" "%NSSM_PATH%" >nul
if %errorlevel% neq 0 (
    call :print_error "Failed to copy NSSM"
    exit /b 1
)

REM Create log file
if not exist "%LOG_FILE%" (
    type nul > "%LOG_FILE%"
)

REM Create and install Windows service using NSSM
call :print_info "Creating Windows service with NSSM..."
"%NSSM_PATH%" install "%SERVICE_NAME%" "%TARGET_EXECUTABLE_PATH%" --config "%CONFIG_FILE%"
if %errorlevel% neq 0 (
    call :print_error "Failed to create service with NSSM"
    exit /b 1
)

REM Set service description and display name
"%NSSM_PATH%" set "%SERVICE_NAME%" Description "%SERVICE_DESCRIPTION%"
"%NSSM_PATH%" set "%SERVICE_NAME%" DisplayName "%SERVICE_DISPLAY_NAME%"

REM Set service to restart on failure
"%NSSM_PATH%" set "%SERVICE_NAME%" AppRestartDelay 10000
"%NSSM_PATH%" set "%SERVICE_NAME%" AppExit Default Restart

REM Set service startup type to automatic
"%NSSM_PATH%" set "%SERVICE_NAME%" Start SERVICE_AUTO_START

REM Start the service
call :print_info "Starting service..."
"%NSSM_PATH%" start "%SERVICE_NAME%"
if %errorlevel% equ 0 (
    call :print_success "DDOGreen installed and started successfully!"
    echo.
    call :print_info "Service Status:"
    sc query "%SERVICE_NAME%"
    echo.
    call :print_info "To view logs: Get-WinEvent -LogName Application -Source ddogreen"
    call :print_info "To stop:      nssm stop ddogreen"
    call :print_info "To restart:   nssm restart ddogreen"
    echo.
    call :print_success "DDOGreen is now managing your system's power automatically!"
) else (
    call :print_error "Service installation failed"
    call :print_info "Check Windows Event Log: Get-WinEvent -LogName Application -Source ddogreen"
    exit /b 1
)
goto :eof

REM Function to uninstall DDOGreen
:uninstall_ddogreen
call :print_info "Uninstalling DDOGreen..."

REM Stop and remove service using NSSM
set NSSM_PATH=%ProgramFiles%\ddosoft\ddogreen\nssm.exe
call :print_info "Stopping and removing service..."
if exist "%NSSM_PATH%" (
    "%NSSM_PATH%" stop "%SERVICE_NAME%" >nul 2>&1
    timeout /t 5 /nobreak >nul
    "%NSSM_PATH%" remove "%SERVICE_NAME%" confirm >nul 2>&1
) else (
    sc stop "%SERVICE_NAME%" >nul 2>&1
    timeout /t 5 /nobreak >nul
    sc delete "%SERVICE_NAME%" >nul 2>&1
)

REM Remove executable and program directory
call :print_info "Removing executables..."
if exist "%TARGET_EXECUTABLE_PATH%" (
    del "%TARGET_EXECUTABLE_PATH%" >nul 2>&1
)
if exist "%NSSM_PATH%" (
    del "%NSSM_PATH%" >nul 2>&1
)
if exist "%ProgramFiles%\ddosoft\ddogreen" (
    rmdir "%ProgramFiles%\ddosoft\ddogreen" >nul 2>&1
)
if exist "%ProgramFiles%\ddosoft" (
    rmdir "%ProgramFiles%\ddosoft" >nul 2>&1
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

call :print_success "DDOGreen has been uninstalled."
goto :eof

REM Function to show status
:show_status
call :print_info "DDOGreen Status:"

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
echo DDOGreen Windows Installer
echo.
echo Usage: installer.bat [OPTION]
echo.
echo Options:
echo   --install     Install DDOGreen service
echo   --uninstall   Uninstall DDOGreen service
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
    call :check_admin || exit /b 1
    call :check_powershell || exit /b 1
    call :install_ddogreen
) else if "%ACTION%"=="uninstall" (
    call :check_admin || exit /b 1
    call :check_powershell || exit /b 1
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