// RepairChecklist.cs
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text.Json;
using System.Text.Json.Serialization;

class Task
{
    [JsonPropertyName("id")] public int Id { get; set; }
    [JsonPropertyName("description")] public string Description { get; set; }
    [JsonPropertyName("completed")] public bool Completed { get; set; }
}

class Category
{
    [JsonPropertyName("name")] public string Name { get; set; }
    [JsonPropertyName("tasks")] public List<Task> Tasks { get; set; } = new List<Task>();
}

class Project
{
    [JsonPropertyName("name")] public string Name { get; set; }
    [JsonPropertyName("categories")] public List<Category> Categories { get; set; } = new List<Category>();
}

class Organizer
{
    private List<Project> projects = new List<Project>();
    private readonly string dataFile = "projects.json";
    private readonly JsonSerializerOptions options = new JsonSerializerOptions { WriteIndented = true };

    public Organizer() { Load(); }

    private void Load()
    {
        if (!File.Exists(dataFile)) return;
        string json = File.ReadAllText(dataFile);
        projects = JsonSerializer.Deserialize<List<Project>>(json) ?? new List<Project>();
    }

    private void Save()
    {
        string json = JsonSerializer.Serialize(projects, options);
        File.WriteAllText(dataFile, json);
    }

    private Project GetProject(string name) => projects.FirstOrDefault(p => p.Name == name);

    public void CreateProject(string name)
    {
        if (GetProject(name) != null)
        {
            Console.WriteLine($"Project '{name}' already exists.");
            return;
        }
        projects.Add(new Project { Name = name });
        Save();
        Console.WriteLine($"Project '{name}' created.");
    }

    public void AddTask(string projectName, string categoryName, string taskDesc)
    {
        var p = GetProject(projectName);
        if (p == null)
        {
            Console.WriteLine($"Project '{projectName}' not found.");
            return;
        }
        var cat = p.Categories.FirstOrDefault(c => c.Name == categoryName);
        if (cat == null)
        {
            cat = new Category { Name = categoryName };
            p.Categories.Add(cat);
        }
        int maxId = p.Categories.SelectMany(c => c.Tasks).Select(t => t.Id).DefaultIfEmpty(0).Max();
        var task = new Task { Id = maxId + 1, Description = taskDesc, Completed = false };
        cat.Tasks.Add(task);
        Save();
        Console.WriteLine($"Task added: [{task.Id}] {taskDesc} (category: {categoryName})");
    }

    public void ListTasks(string projectName)
    {
        var p = GetProject(projectName);
        if (p == null)
        {
            Console.WriteLine($"Project '{projectName}' not found.");
            return;
        }
        if (!p.Categories.Any())
        {
            Console.WriteLine("No categories yet.");
            return;
        }
        Console.WriteLine($"\n📋 Checklist for '{p.Name}':");
        foreach (var c in p.Categories)
        {
            Console.WriteLine($"\n\u001b[36m📁 {c.Name}\u001b[0m");
            if (!c.Tasks.Any())
            {
                Console.WriteLine("  (no tasks)");
            }
            else
            {
                foreach (var t in c.Tasks)
                {
                    string status = t.Completed ? "\u001b[32m✓\u001b[0m" : "\u001b[31m✗\u001b[0m";
                    Console.WriteLine($"  [{t.Id}] {status} {t.Description}");
                }
            }
        }
    }

    public void CheckTask(string projectName, int taskId)
    {
        var p = GetProject(projectName);
        if (p == null)
        {
            Console.WriteLine($"Project '{projectName}' not found.");
            return;
        }
        foreach (var c in p.Categories)
        {
            foreach (var t in c.Tasks)
            {
                if (t.Id == taskId)
                {
                    if (t.Completed)
                        Console.WriteLine($"Task [{taskId}] already completed.");
                    else
                    {
                        t.Completed = true;
                        Save();
                        Console.WriteLine($"Task [{taskId}] marked as completed.");
                    }
                    return;
                }
            }
        }
        Console.WriteLine($"Task with ID {taskId} not found in project '{projectName}'.");
    }

    public void Progress(string projectName)
    {
        var p = GetProject(projectName);
        if (p == null)
        {
            Console.WriteLine($"Project '{projectName}' not found.");
            return;
        }
        int total = p.Categories.Sum(c => c.Tasks.Count);
        int done = p.Categories.Sum(c => c.Tasks.Count(t => t.Completed));
        if (total == 0)
        {
            Console.WriteLine("No tasks yet.");
            return;
        }
        double pct = (double)done / total * 100;
        Console.WriteLine($"\n📊 Progress for '{p.Name}': {done}/{total} tasks completed ({pct:F1}%)");
        int barLen = 30;
        int filled = (int)(barLen * done / total);
        string bar = new string('█', filled) + new string('░', barLen - filled);
        Console.WriteLine($"[{bar}] {pct:F1}%");
    }

    public void ExportHTML(string projectName)
    {
        var p = GetProject(projectName);
        if (p == null)
        {
            Console.WriteLine($"Project '{projectName}' not found.");
            return;
        }
        var html = $@"<!DOCTYPE html>
<html><head><title>Checklist - {p.Name}</title>
<style>body{{font-family:sans-serif;margin:30px;background:#f5f5f5;}}
h1{{color:#333;}} .cat{{background:#fff;border-radius:8px;padding:15px;margin:10px 0;box-shadow:0 2px 4px rgba(0,0,0,0.1);}}
.task{{padding:5px 0;border-bottom:1px solid #eee;}}
.done{{color:green;}} .pending{{color:red;}}
</style></head><body>
<h1>📋 Checklist: {p.Name}</h1>";
        foreach (var c in p.Categories)
        {
            html += $"<div class='cat'><h2>📁 {c.Name}</h2>";
            if (!c.Tasks.Any())
                html += "<p><em>No tasks</em></p>";
            else
                foreach (var t in c.Tasks)
                {
                    string cls = t.Completed ? "done" : "pending";
                    string mark = t.Completed ? "✓" : "✗";
                    html += $"<div class='task {cls}'><span>{mark}</span> {t.Description}</div>";
                }
            html += "</div>";
        }
        html += "</body></html>";
        string filename = p.Name.Replace(" ", "_") + "_checklist.html";
        File.WriteAllText(filename, html);
        Console.WriteLine($"Report exported to {filename}");
    }
}

class Program
{
    static void Main(string[] args)
    {
        if (args.Length < 1)
        {
            Console.WriteLine("Usage: RepairChecklist <command> [options]");
            return;
        }
        var app = new Organizer();
        string cmd = args[0];
        switch (cmd)
        {
            case "create":
                if (args.Length < 2) { Console.WriteLine("create <name>"); return; }
                app.CreateProject(args[1]);
                break;
            case "add":
                if (args.Length < 4) { Console.WriteLine("add <project> <category> <task>"); return; }
                app.AddTask(args[1], args[2], args[3]);
                break;
            case "list":
                if (args.Length < 2) { Console.WriteLine("list <project>"); return; }
                app.ListTasks(args[1]);
                break;
            case "check":
                if (args.Length < 3) { Console.WriteLine("check <project> <task_id>"); return; }
                app.CheckTask(args[1], int.Parse(args[2]));
                break;
            case "progress":
                if (args.Length < 2) { Console.WriteLine("progress <project>"); return; }
                app.Progress(args[1]);
                break;
            case "export":
                if (args.Length < 2) { Console.WriteLine("export <project>"); return; }
                app.ExportHTML(args[1]);
                break;
            default:
                Console.WriteLine("Unknown command.");
                break;
        }
    }
}
