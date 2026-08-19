// RepairChecklist.java
import java.io.*;
import java.nio.file.*;
import java.util.*;
import com.google.gson.*;

class Task {
    int id;
    String description;
    boolean completed;
    // Gson needs default constructor
    public Task() {}
    public Task(int id, String description, boolean completed) {
        this.id = id;
        this.description = description;
        this.completed = completed;
    }
}

class Category {
    String name;
    List<Task> tasks = new ArrayList<>();
    public Category() {}
    public Category(String name) { this.name = name; }
}

class Project {
    String name;
    List<Category> categories = new ArrayList<>();
    public Project() {}
    public Project(String name) { this.name = name; }
}

class Organizer {
    private List<Project> projects = new ArrayList<>();
    private final String dataFile = "projects.json";
    private final Gson gson = new GsonBuilder().setPrettyPrinting().create();

    public Organizer() { load(); }

    private void load() {
        try {
            Path path = Paths.get(dataFile);
            if (Files.exists(path)) {
                String json = new String(Files.readAllBytes(path));
                Project[] arr = gson.fromJson(json, Project[].class);
                projects = Arrays.asList(arr);
            }
        } catch (Exception e) {}
    }

    private void save() {
        try {
            Files.write(Paths.get(dataFile), gson.toJson(projects).getBytes());
        } catch (Exception e) {}
    }

    private Project getProject(String name) {
        for (Project p : projects) if (p.name.equals(name)) return p;
        return null;
    }

    public void createProject(String name) {
        if (getProject(name) != null) {
            System.out.printf("Project '%s' already exists.\n", name);
            return;
        }
        projects.add(new Project(name));
        save();
        System.out.printf("Project '%s' created.\n", name);
    }

    public void addTask(String projectName, String categoryName, String taskDesc) {
        Project p = getProject(projectName);
        if (p == null) {
            System.out.printf("Project '%s' not found.\n", projectName);
            return;
        }
        Category cat = null;
        for (Category c : p.categories) {
            if (c.name.equals(categoryName)) { cat = c; break; }
        }
        if (cat == null) {
            cat = new Category(categoryName);
            p.categories.add(cat);
        }
        int maxId = 0;
        for (Category c : p.categories) {
            for (Task t : c.tasks) {
                if (t.id > maxId) maxId = t.id;
            }
        }
        Task task = new Task(maxId + 1, taskDesc, false);
        cat.tasks.add(task);
        save();
        System.out.printf("Task added: [%d] %s (category: %s)\n", task.id, taskDesc, categoryName);
    }

    public void listTasks(String projectName) {
        Project p = getProject(projectName);
        if (p == null) {
            System.out.printf("Project '%s' not found.\n", projectName);
            return;
        }
        if (p.categories.isEmpty()) {
            System.out.println("No categories yet.");
            return;
        }
        System.out.printf("\n📋 Checklist for '%s':\n", p.name);
        for (Category c : p.categories) {
            System.out.printf("\n\033[36m📁 %s\033[0m\n", c.name);
            if (c.tasks.isEmpty()) {
                System.out.println("  (no tasks)");
            } else {
                for (Task t : c.tasks) {
                    String status = t.completed ? "\033[32m✓\033[0m" : "\033[31m✗\033[0m";
                    System.out.printf("  [%d] %s %s\n", t.id, status, t.description);
                }
            }
        }
    }

    public void checkTask(String projectName, int taskId) {
        Project p = getProject(projectName);
        if (p == null) {
            System.out.printf("Project '%s' not found.\n", projectName);
            return;
        }
        for (Category c : p.categories) {
            for (Task t : c.tasks) {
                if (t.id == taskId) {
                    if (t.completed) {
                        System.out.printf("Task [%d] already completed.\n", taskId);
                    } else {
                        t.completed = true;
                        save();
                        System.out.printf("Task [%d] marked as completed.\n", taskId);
                    }
                    return;
                }
            }
        }
        System.out.printf("Task with ID %d not found in project '%s'.\n", taskId, projectName);
    }

    public void progress(String projectName) {
        Project p = getProject(projectName);
        if (p == null) {
            System.out.printf("Project '%s' not found.\n", projectName);
            return;
        }
        int total = 0, done = 0;
        for (Category c : p.categories) {
            for (Task t : c.tasks) {
                total++;
                if (t.completed) done++;
            }
        }
        if (total == 0) {
            System.out.println("No tasks yet.");
            return;
        }
        double pct = (double) done / total * 100;
        System.out.printf("\n📊 Progress for '%s': %d/%d tasks completed (%.1f%%)\n", p.name, done, total, pct);
        int barLen = 30;
        int filled = (int)(barLen * done / total);
        String bar = "█".repeat(filled) + "░".repeat(barLen - filled);
        System.out.printf("[%s] %.1f%%\n", bar, pct);
    }

    public void exportHTML(String projectName) throws IOException {
        Project p = getProject(projectName);
        if (p == null) {
            System.out.printf("Project '%s' not found.\n", projectName);
            return;
        }
        StringBuilder html = new StringBuilder();
        html.append("<!DOCTYPE html>\n<html><head><title>Checklist - ").append(p.name).append("</title>\n");
        html.append("<style>body{font-family:sans-serif;margin:30px;background:#f5f5f5;}\n");
        html.append("h1{color:#333;} .cat{background:#fff;border-radius:8px;padding:15px;margin:10px 0;box-shadow:0 2px 4px rgba(0,0,0,0.1);}\n");
        html.append(".task{padding:5px 0;border-bottom:1px solid #eee;}\n");
        html.append(".done{color:green;} .pending{color:red;}\n");
        html.append("</style></head><body>\n");
        html.append("<h1>📋 Checklist: ").append(p.name).append("</h1>\n");
        for (Category c : p.categories) {
            html.append("<div class='cat'><h2>📁 ").append(c.name).append("</h2>");
            if (c.tasks.isEmpty()) {
                html.append("<p><em>No tasks</em></p>");
            } else {
                for (Task t : c.tasks) {
                    String cls = t.completed ? "done" : "pending";
                    String mark = t.completed ? "✓" : "✗";
                    html.append("<div class='task ").append(cls).append("'><span>").append(mark).append("</span> ").append(t.description).append("</div>");
                }
            }
            html.append("</div>");
        }
        html.append("</body></html>");
        String filename = p.name.replace(" ", "_") + "_checklist.html";
        Files.write(Paths.get(filename), html.toString().getBytes());
        System.out.println("Report exported to " + filename);
    }

    public static void main(String[] args) throws Exception {
        if (args.length < 1) {
            System.out.println("Usage: RepairChecklist <command> [options]");
            return;
        }
        Organizer app = new Organizer();
        String cmd = args[0];
        switch (cmd) {
            case "create":
                if (args.length < 2) { System.out.println("create <name>"); return; }
                app.createProject(args[1]);
                break;
            case "add":
                if (args.length < 4) { System.out.println("add <project> <category> <task>"); return; }
                app.addTask(args[1], args[2], args[3]);
                break;
            case "list":
                if (args.length < 2) { System.out.println("list <project>"); return; }
                app.listTasks(args[1]);
                break;
            case "check":
                if (args.length < 3) { System.out.println("check <project> <task_id>"); return; }
                app.checkTask(args[1], Integer.parseInt(args[2]));
                break;
            case "progress":
                if (args.length < 2) { System.out.println("progress <project>"); return; }
                app.progress(args[1]);
                break;
            case "export":
                if (args.length < 2) { System.out.println("export <project>"); return; }
                app.exportHTML(args[1]);
                break;
            default:
                System.out.println("Unknown command.");
        }
    }
}
