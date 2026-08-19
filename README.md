🛠️ Repair Checklist Generator — Multi‑Language Task Manager for Renovation Projects
8 languages, one powerful checklist tool – organize your renovation tasks by categories, track progress, and stay on schedule.

✨ Features
🏗️ Create projects – name your renovation (e.g., "Apartment", "Car", "Office")

📂 Organize by categories – group tasks (e.g., "Plumbing", "Electrical", "Finishing")

✅ Add tasks with descriptions – each task gets a unique ID

📊 Track progress – see percentage of completed tasks per project

🎨 Color‑coded output – completed tasks in green, pending in red

💾 Persistent storage – all data saved in a local JSON file

📤 Export – generate an HTML report of the checklist (optional)

🚀 Quick Start
All implementations share the same CLI interface:

bash
# Create a new project
<program> create "Apartment Renovation"

# Add a category and a task
<program> add "Apartment Renovation" "Plumbing" "Replace kitchen faucet"

# List all tasks with IDs and status
<program> list "Apartment Renovation"

# Mark a task as completed (use the task ID from the list)
<program> check "Apartment Renovation" 1

# Show overall progress
<program> progress "Apartment Renovation"

# Export to HTML report
<program> export "Apartment Renovation"
Commands:

create <name> – create a new project

add <project> <category> <task> – add a task to a category

list <project> – show all categories and tasks with status

check <project> <task_id> – mark a task as done

progress <project> – show completion percentage

export <project> – generate report.html (optional)

📁 Repository Structure
text
.
├── README.md
├── python/
│   └── repair_checklist.py
├── go/
│   └── repair_checklist.go
├── javascript/
│   └── repair_checklist.js
├── ruby/
│   └── repair_checklist.rb
├── php/
│   └── repair_checklist.php
├── java/
│   └── RepairChecklist.java
├── csharp/
│   └── RepairChecklist.cs
└── cpp/
    └── repair_checklist.cpp
