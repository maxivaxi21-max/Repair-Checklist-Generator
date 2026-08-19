# repair_checklist.py
import sys, os, json, argparse
from datetime import datetime
try:
    from colorama import init, Fore, Style
    init()
    COLORS = True
except ImportError:
    COLORS = False
    Fore = Style = type('', (), {'RESET_ALL':'', 'GREEN':'', 'RED':'', 'YELLOW':'', 'CYAN':''})()

DATA_FILE = "projects.json"

class Task:
    def __init__(self, description, completed=False, task_id=None):
        self.id = task_id or 0
        self.description = description
        self.completed = completed

    def to_dict(self):
        return {"id": self.id, "description": self.description, "completed": self.completed}

    @classmethod
    def from_dict(cls, data):
        return cls(data["description"], data["completed"], data["id"])

class Category:
    def __init__(self, name):
        self.name = name
        self.tasks = []

    def to_dict(self):
        return {"name": self.name, "tasks": [t.to_dict() for t in self.tasks]}

    @classmethod
    def from_dict(cls, data):
        c = cls(data["name"])
        c.tasks = [Task.from_dict(t) for t in data["tasks"]]
        return c

class Project:
    def __init__(self, name):
        self.name = name
        self.categories = []

    def to_dict(self):
        return {"name": self.name, "categories": [c.to_dict() for c in self.categories]}

    @classmethod
    def from_dict(cls, data):
        p = cls(data["name"])
        p.categories = [Category.from_dict(c) for c in data["categories"]]
        return p

class Organizer:
    def __init__(self):
        self.projects = []
        self.load()

    def load(self):
        if os.path.exists(DATA_FILE):
            with open(DATA_FILE, "r") as f:
                data = json.load(f)
                self.projects = [Project.from_dict(p) for p in data]

    def save(self):
        with open(DATA_FILE, "w") as f:
            json.dump([p.to_dict() for p in self.projects], f, indent=2)

    def get_project(self, name):
        for p in self.projects:
            if p.name == name:
                return p
        return None

    def create_project(self, name):
        if self.get_project(name):
            print(f"Project '{name}' already exists.")
            return
        p = Project(name)
        self.projects.append(p)
        self.save()
        print(f"Project '{name}' created.")

    def add_task(self, project_name, category_name, task_desc):
        p = self.get_project(project_name)
        if not p:
            print(f"Project '{project_name}' not found.")
            return
        cat = None
        for c in p.categories:
            if c.name == category_name:
                cat = c
                break
        if not cat:
            cat = Category(category_name)
            p.categories.append(cat)
        # Assign ID: max existing + 1
        max_id = 0
        for c in p.categories:
            for t in c.tasks:
                if t.id > max_id:
                    max_id = t.id
        task = Task(task_desc, False, max_id + 1)
        cat.tasks.append(task)
        self.save()
        print(f"Task added: [{task.id}] {task_desc} (category: {category_name})")

    def list_tasks(self, project_name):
        p = self.get_project(project_name)
        if not p:
            print(f"Project '{project_name}' not found.")
            return
        if not p.categories:
            print("No categories yet.")
            return
        print(f"\n📋 Checklist for '{p.name}':")
        for c in p.categories:
            print(f"\n{Fore.CYAN}📁 {c.name}{Style.RESET_ALL}")
            if not c.tasks:
                print("  (no tasks)")
            else:
                for t in c.tasks:
                    status = f"{Fore.GREEN}✓{Style.RESET_ALL}" if t.completed else f"{Fore.RED}✗{Style.RESET_ALL}"
                    print(f"  [{t.id}] {status} {t.description}")

    def check_task(self, project_name, task_id):
        p = self.get_project(project_name)
        if not p:
            print(f"Project '{project_name}' not found.")
            return
        for c in p.categories:
            for t in c.tasks:
                if t.id == task_id:
                    if t.completed:
                        print(f"Task [{task_id}] already completed.")
                    else:
                        t.completed = True
                        self.save()
                        print(f"Task [{task_id}] marked as completed.")
                    return
        print(f"Task with ID {task_id} not found in project '{project_name}'.")

    def progress(self, project_name):
        p = self.get_project(project_name)
        if not p:
            print(f"Project '{project_name}' not found.")
            return
        total = 0
        done = 0
        for c in p.categories:
            for t in c.tasks:
                total += 1
                if t.completed:
                    done += 1
        if total == 0:
            print("No tasks yet.")
            return
        pct = (done / total) * 100
        print(f"\n📊 Progress for '{p.name}': {done}/{total} tasks completed ({pct:.1f}%)")
        # Show bar
        bar_len = 30
        filled = int(bar_len * done / total)
        bar = '█' * filled + '░' * (bar_len - filled)
        print(f"[{bar}] {pct:.1f}%")

    def export_html(self, project_name):
        p = self.get_project(project_name)
        if not p:
            print(f"Project '{project_name}' not found.")
            return
        html = f"""<!DOCTYPE html>
<html><head><title>Checklist - {p.name}</title>
<style>body{{font-family:sans-serif;margin:30px;background:#f5f5f5;}}
h1{{color:#333;}} .cat{{background:#fff;border-radius:8px;padding:15px;margin:10px 0;box-shadow:0 2px 4px rgba(0,0,0,0.1);}}
.task{{padding:5px 0;border-bottom:1px solid #eee;}}
.done{{color:green;}} .pending{{color:red;}}
</style></head><body>
<h1>📋 Checklist: {p.name}</h1>
"""
        for c in p.categories:
            html += f"<div class='cat'><h2>📁 {c.name}</h2>"
            if not c.tasks:
                html += "<p><em>No tasks</em></p>"
            else:
                for t in c.tasks:
                    cls = "done" if t.completed else "pending"
                    mark = "✓" if t.completed else "✗"
                    html += f"<div class='task {cls}'><span>{mark}</span> {t.description}</div>"
            html += "</div>"
        html += "</body></html>"
        filename = f"{p.name.replace(' ', '_')}_checklist.html"
        with open(filename, "w") as f:
            f.write(html)
        print(f"Report exported to {filename}")

def main():
    parser = argparse.ArgumentParser(description="Repair Checklist Generator")
    subparsers = parser.add_subparsers(dest="cmd", required=True)

    create_parser = subparsers.add_parser("create")
    create_parser.add_argument("name")

    add_parser = subparsers.add_parser("add")
    add_parser.add_argument("project")
    add_parser.add_argument("category")
    add_parser.add_argument("task")

    list_parser = subparsers.add_parser("list")
    list_parser.add_argument("project")

    check_parser = subparsers.add_parser("check")
    check_parser.add_argument("project")
    check_parser.add_argument("task_id", type=int)

    progress_parser = subparsers.add_parser("progress")
    progress_parser.add_argument("project")

    export_parser = subparsers.add_parser("export")
    export_parser.add_argument("project")

    args = parser.parse_args()
    app = Organizer()

    if args.cmd == "create":
        app.create_project(args.name)
    elif args.cmd == "add":
        app.add_task(args.project, args.category, args.task)
    elif args.cmd == "list":
        app.list_tasks(args.project)
    elif args.cmd == "check":
        app.check_task(args.project, args.task_id)
    elif args.cmd == "progress":
        app.progress(args.project)
    elif args.cmd == "export":
        app.export_html(args.project)

if __name__ == "__main__":
    main()
