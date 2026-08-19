// repair_checklist.cpp
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::json;

struct Task {
    int id;
    string description;
    bool completed;
};

struct Category {
    string name;
    vector<Task> tasks;
};

struct Project {
    string name;
    vector<Category> categories;
};

class Organizer {
private:
    vector<Project> projects;
    string dataFile = "projects.json";

    void load() {
        ifstream f(dataFile);
        if (!f.is_open()) return;
        json j;
        f >> j;
        for (auto& pj : j) {
            Project p;
            p.name = pj["name"];
            for (auto& cj : pj["categories"]) {
                Category c;
                c.name = cj["name"];
                for (auto& tj : cj["tasks"]) {
                    Task t;
                    t.id = tj["id"];
                    t.description = tj["description"];
                    t.completed = tj["completed"];
                    c.tasks.push_back(t);
                }
                p.categories.push_back(c);
            }
            projects.push_back(p);
        }
    }

    void save() {
        json j = json::array();
        for (auto& p : projects) {
            json categories = json::array();
            for (auto& c : p.categories) {
                json tasks = json::array();
                for (auto& t : c.tasks) {
                    tasks.push_back({{"id", t.id}, {"description", t.description}, {"completed", t.completed}});
                }
                categories.push_back({{"name", c.name}, {"tasks", tasks}});
            }
            j.push_back({{"name", p.name}, {"categories", categories}});
        }
        ofstream f(dataFile);
        f << setw(2) << j << endl;
    }

    Project* getProject(const string& name) {
        for (auto& p : projects)
            if (p.name == name) return &p;
        return nullptr;
    }

public:
    Organizer() { load(); }

    void createProject(const string& name) {
        if (getProject(name)) {
            cout << "Project '" << name << "' already exists.\n";
            return;
        }
        Project p; p.name = name;
        projects.push_back(p);
        save();
        cout << "Project '" << name << "' created.\n";
    }

    void addTask(const string& projectName, const string& categoryName, const string& taskDesc) {
        Project* p = getProject(projectName);
        if (!p) {
            cout << "Project '" << projectName << "' not found.\n";
            return;
        }
        Category* cat = nullptr;
        for (auto& c : p->categories)
            if (c.name == categoryName) { cat = &c; break; }
        if (!cat) {
            Category c; c.name = categoryName;
            p->categories.push_back(c);
            cat = &p->categories.back();
        }
        int maxId = 0;
        for (auto& c : p->categories)
            for (auto& t : c.tasks)
                if (t.id > maxId) maxId = t.id;
        Task task{maxId + 1, taskDesc, false};
        cat->tasks.push_back(task);
        save();
        cout << "Task added: [" << task.id << "] " << taskDesc << " (category: " << categoryName << ")\n";
    }

    void listTasks(const string& projectName) {
        Project* p = getProject(projectName);
        if (!p) {
            cout << "Project '" << projectName << "' not found.\n";
            return;
        }
        if (p->categories.empty()) {
            cout << "No categories yet.\n";
            return;
        }
        cout << "\n📋 Checklist for '" << p->name << "':\n";
        for (auto& c : p->categories) {
            cout << "\n\033[36m📁 " << c.name << "\033[0m\n";
            if (c.tasks.empty()) {
                cout << "  (no tasks)\n";
            } else {
                for (auto& t : c.tasks) {
                    string status = t.completed ? "\033[32m✓\033[0m" : "\033[31m✗\033[0m";
                    cout << "  [" << t.id << "] " << status << " " << t.description << "\n";
                }
            }
        }
    }

    void checkTask(const string& projectName, int taskId) {
        Project* p = getProject(projectName);
        if (!p) {
            cout << "Project '" << projectName << "' not found.\n";
            return;
        }
        for (auto& c : p->categories) {
            for (auto& t : c.tasks) {
                if (t.id == taskId) {
                    if (t.completed)
                        cout << "Task [" << taskId << "] already completed.\n";
                    else {
                        t.completed = true;
                        save();
                        cout << "Task [" << taskId << "] marked as completed.\n";
                    }
                    return;
                }
            }
        }
        cout << "Task with ID " << taskId << " not found in project '" << projectName << "'.\n";
    }

    void progress(const string& projectName) {
        Project* p = getProject(projectName);
        if (!p) {
            cout << "Project '" << projectName << "' not found.\n";
            return;
        }
        int total = 0, done = 0;
        for (auto& c : p->categories)
            for (auto& t : c.tasks) {
                total++;
                if (t.completed) done++;
            }
        if (total == 0) {
            cout << "No tasks yet.\n";
            return;
        }
        double pct = (double)done / total * 100;
        cout << "\n📊 Progress for '" << p->name << "': " << done << "/" << total << " tasks completed (" << fixed << setprecision(1) << pct << "%)\n";
        int barLen = 30;
        int filled = (int)(barLen * done / total);
        string bar(filled, '█');
        bar.append(barLen - filled, '░');
        cout << "[" << bar << "] " << pct << "%\n";
    }

    void exportHTML(const string& projectName) {
        Project* p = getProject(projectName);
        if (!p) {
            cout << "Project '" << projectName << "' not found.\n";
            return;
        }
        string html = "<!DOCTYPE html>\n<html><head><title>Checklist - " + p->name + "</title>\n";
        html += "<style>body{font-family:sans-serif;margin:30px;background:#f5f5f5;}\n";
        html += "h1{color:#333;} .cat{background:#fff;border-radius:8px;padding:15px;margin:10px 0;box-shadow:0 2px 4px rgba(0,0,0,0.1);}\n";
        html += ".task{padding:5px 0;border-bottom:1px solid #eee;}\n";
        html += ".done{color:green;} .pending{color:red;}\n";
        html += "</style></head><body>\n";
        html += "<h1>📋 Checklist: " + p->name + "</h1>\n";
        for (auto& c : p->categories) {
            html += "<div class='cat'><h2>📁 " + c.name + "</h2>";
            if (c.tasks.empty()) {
                html += "<p><em>No tasks</em></p>";
            } else {
                for (auto& t : c.tasks) {
                    string cls = t.completed ? "done" : "pending";
                    string mark = t.completed ? "✓" : "✗";
                    html += "<div class='task " + cls + "'><span>" + mark + "</span> " + t.description + "</div>";
                }
            }
            html += "</div>";
        }
        html += "</body></html>";
        string filename = p->name;
        replace(filename.begin(), filename.end(), ' ', '_');
        filename += "_checklist.html";
        ofstream f(filename);
        f << html;
        cout << "Report exported to " << filename << "\n";
    }
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: repair_checklist <command> [options]\n";
        return 1;
    }
    Organizer app;
    string cmd = argv[1];
    if (cmd == "create") {
        if (argc < 3) { cerr << "create <name>\n"; return 1; }
        app.createProject(argv[2]);
    } else if (cmd == "add") {
        if (argc < 5) { cerr << "add <project> <category> <task>\n"; return 1; }
        app.addTask(argv[2], argv[3], argv[4]);
    } else if (cmd == "list") {
        if (argc < 3) { cerr << "list <project>\n"; return 1; }
        app.listTasks(argv[2]);
    } else if (cmd == "check") {
        if (argc < 4) { cerr << "check <project> <task_id>\n"; return 1; }
        app.checkTask(argv[2], stoi(argv[3]));
    } else if (cmd == "progress") {
        if (argc < 3) { cerr << "progress <project>\n"; return 1; }
        app.progress(argv[2]);
    } else if (cmd == "export") {
        if (argc < 3) { cerr << "export <project>\n"; return 1; }
        app.exportHTML(argv[2]);
    } else {
        cerr << "Unknown command.\n";
    }
    return 0;
}
