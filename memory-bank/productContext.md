# Product Context - ddotlp

## Why This Project Exists

### Problem Statement
Modern laptops, especially ThinkPads, benefit greatly from intelligent power management, but existing solutions have limitations:

1. **Manual Switching**: Users must manually switch between performance and battery modes
2. **X11 Dependencies**: Many activity monitors require GUI/X11, limiting server use
3. **Complex Setup**: Existing solutions often require complex configuration
4. **Inconsistent Behavior**: Manual switching leads to forgotten mode changes and poor battery life

### Solution Approach
ddotlp provides an automated, intelligent solution that:
- **Learns from Usage**: Monitors actual CPU activity patterns
- **Universal Compatibility**: Works on any Linux system (desktop, server, headless)
- **Zero Configuration**: Works out-of-the-box with sensible defaults
- **Transparent Operation**: Runs silently in background with detailed logging when needed

## How It Should Work

### User Experience Goals
1. **Invisible Operation**: Users should not need to think about power management
2. **Intelligent Decisions**: System should switch modes based on actual activity, not just user input
3. **Reliable Performance**: When working, system should be in auto mode
4. **Extended Battery**: When idle, system should maximize battery life
5. **Easy Monitoring**: Users can check logs to understand system behavior

### Operational Flow
```
System Boot → Service Starts → Monitor CPU Usage → Detect Activity Patterns
     ↓
Active CPU Usage (>15%) → Auto Mode (tlp start)
     ↓
Idle CPU Usage (<15% for 5+ min) → Battery Mode (tlp bat)
     ↓
Activity Resumes → Auto Mode → Continue Monitoring
```

### Business Value
1. **Extended Battery Life**: Automatic battery mode when system is idle
2. **Better Performance**: Automatic auto mode when actively working
3. **Reduced Complexity**: No manual power management needed
4. **Universal Deployment**: Works across different environments and setups

## Target Scenarios

### Primary Use Cases
1. **Development Workstations**: Developers who alternate between coding and meetings
2. **Business Laptops**: Users who present, then let laptop sit idle
3. **Home Users**: Mixed usage patterns throughout the day
4. **Server Management**: Headless systems that need intelligent power management

### Secondary Use Cases
1. **Lab Environments**: Multiple systems needing consistent power management
2. **Educational Settings**: Computer labs with varying usage patterns
3. **Remote Workers**: Mobile setups with unpredictable power access

## Integration Requirements

### TLP Integration
- Must work with existing TLP installations
- Should not interfere with manual TLP usage
- Must handle TLP command failures gracefully
- Should capture and log TLP output for debugging

### System Integration
- Proper systemd service with auto-start capability
- Standard Linux logging and monitoring integration
- Configuration through standard config files
- Log rotation and management

### User Integration
- Minimal configuration required
- Clear documentation and troubleshooting guides
- Easy installation and removal
- Status monitoring through standard Linux tools

## Expected Outcomes
1. **Improved Battery Life**: 20-30% improvement in battery duration during typical mixed usage
2. **Better Performance**: Full performance available when needed without manual intervention
3. **Reduced User Effort**: Zero daily management overhead
4. **System Reliability**: Stable, long-running service operation
5. **Clear Monitoring**: Complete visibility into power management decisions
