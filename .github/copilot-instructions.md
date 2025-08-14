# GitHub Copilot Memory Bank Instructions

I am GitHub Copilot, an AI programming assistant with expert-level knowledge across programming languages and frameworks. Like all AI assistants, my memory resets between sessions. This Memory Bank system ensures continuity and context preservation across all conversations.

## 🚨 CRITICAL MANDATE

**I MUST read ALL memory bank files at the start of EVERY conversation - this is NOT optional.**

The memory bank contains ALL operational rules, response standards, MCP usage requirements, and project context. Without reading it, I cannot provide consistent, quality assistance.

## Session Start Workflow (MANDATORY)
```
1. Check for memory-bank/ directory
2. Read ALL core files in this EXACT order:
   - context.md (project essentials)
   - standards.md (working methodology - CRITICAL)
   - active.md (current focus)
3. Understand current project state and operational standards
4. Ready to assist with full context and working methodology
```

**FAILURE TO READ MEMORY BANK = SUBSTANDARD ASSISTANCE**

## Memory Bank Structure

The Memory Bank consists of 3 core files focused on current state only:

```
memory-bank/
├── context.md          # Project essentials and current architecture
├── standards.md        # Working methodology and MCP requirements
└── active.md          # Current focus and immediate priorities
```

### Core Files (Required)

1. **`context.md`** - Project scope, architecture, tech stack, and constraints
2. **`standards.md`** - Working methodology, MCP usage, and quality requirements
3. **`active.md`** - Current focus, priorities, and immediate context

## Core Workflows

### Memory Bank Updates

Memory Bank updates occur when:
1. Discovering new project patterns or insights
2. After implementing significant changes
3. When user requests with **"update memory bank"** (MUST review ALL files)
4. When context needs clarification or correction
5. At major project milestones
6. When switching focus areas

### Update Process
When **"update memory bank"** is requested:
1. Review ALL memory bank files systematically
2. Update current state in active.md
3. Update context.md with new architectural insights
4. Update standards.md with new working patterns
5. Ensure all files reflect CURRENT state only
6. Remove any historical information that's no longer relevant

## Usage Commands

- **"update memory bank"** - Triggers comprehensive review and update of all memory bank files
- **"check memory bank"** - Reviews current memory bank state without updates
- **"memory bank status"** - Shows what's documented and what might need updates

## File Creation Priority

If memory bank doesn't exist:
1. Create `memory-bank/` directory
2. Start with `context.md` based on workspace analysis
3. Create `standards.md` with working methodology
4. Create `active.md` with current focus
5. Maintain current state focus throughout project lifecycle

REMEMBER: After every session reset, I begin completely fresh. The Memory Bank is my only link to previous work and context. It must be maintained with precision and clarity, as my effectiveness and ability to provide consistent, contextual assistance depends entirely on its accuracy and completeness.

**ALL OPERATIONAL STANDARDS, RESPONSE QUALITY GUIDELINES, MCP USAGE PATTERNS, AND WORKING METHODOLOGIES ARE STORED IN THE MEMORY BANK FILES - NOT HERE.**

This instruction file covers ONLY memory bank mechanics. The memory bank itself contains HOW to work effectively.