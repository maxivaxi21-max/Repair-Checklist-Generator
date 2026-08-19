// repair_checklist.go
package main

import (
	"encoding/json"
	"fmt"
	"os"
	"strconv"
	"strings"
)

type Task struct {
	ID          int    `json:"id"`
	Description string `json:"description"`
	Completed   bool   `json:"completed"`
}

type Category struct {
	Name  string `json:"name"`
	Tasks []Task `json:"tasks"`
}

type Project struct {
	Name       string     `json:"name"`
	Categories []Category `json:"categories"`
}

type Organizer struct {
	Projects []Project `json:"projects"`
	File     string
}

func NewOrganizer(file string) *Organizer {
	o := &Organizer{File: file}
	o.load()
	return o
}

func (o *Organizer) load() {
	data, err := os.ReadFile(o.File)
	if err != nil {
		return
	}
	json.Unmarshal(data, o)
}

func (o *Organizer) save() {
	data, _ := json.MarshalIndent(o, "", "  ")
	os.WriteFile(o.File, data, 0644)
}

func (o *Organizer) getProject(name string) *Project {
	for i := range o.Projects {
		if o.Projects[i].Name == name {
			return &o.Projects[i]
		}
	}
	return nil
}

func (o *Organizer) createProject(name string) {
	if o.getProject(name) != nil {
		fmt.Printf("Project '%s' already exists.\n", name)
		return
	}
	o.Projects = append(o.Projects, Project{Name: name})
	o.save()
	fmt.Printf("Project '%s' created.\n", name)
}

func (o *Organizer) addTask(projectName, categoryName, taskDesc string) {
	p := o.getProject(projectName)
	if p == nil {
		fmt.Printf("Project '%s' not found.\n", projectName)
		return
	}
	// find category
	var cat *Category
	for i := range p.Categories {
		if p.Categories[i].Name == categoryName {
			cat = &p.Categories[i]
			break
		}
	}
	if cat == nil {
		p.Categories = append(p.Categories, Category{Name: categoryName})
		cat = &p.Categories[len(p.Categories)-1]
	}
	// assign ID
	maxID := 0
	for _, c := range p.Categories {
		for _, t := range c.Tasks {
			if t.ID > maxID {
				maxID = t.ID
			}
		}
	}
	task := Task{ID: maxID + 1, Description: taskDesc, Completed: false}
	cat.Tasks = append(cat.Tasks, task)
	o.save()
	fmt.Printf("Task added: [%d] %s (category: %s)\n", task.ID, taskDesc, categoryName)
}

func (o *Organizer) listTasks(projectName string) {
	p := o.getProject(projectName)
	if p == nil {
		fmt.Printf("Project '%s' not found.\n", projectName)
		return
	}
	if len(p.Categories) == 0 {
		fmt.Println("No categories yet.")
		return
	}
	fmt.Printf("\n📋 Checklist for '%s':\n", p.Name)
	for _, c := range p.Categories {
		fmt.Printf("\n\033[36m📁 %s\033[0m\n", c.Name)
		if len(c.Tasks) == 0 {
			fmt.Println("  (no tasks)")
		} else {
			for _, t := range c.Tasks {
				status := "\033[32m✓\033[0m"
				if !t.Completed {
					status = "\033[31m✗\033[0m"
				}
				fmt.Printf("  [%d] %s %s\n", t.ID, status, t.Description)
			}
		}
	}
}

func (o *Organizer) checkTask(projectName string, taskID int) {
	p := o.getProject(projectName)
	if p == nil {
		fmt.Printf("Project '%s' not found.\n", projectName)
		return
	}
	for i := range p.Categories {
		for j := range p.Categories[i].Tasks {
			if p.Categories[i].Tasks[j].ID == taskID {
				if p.Categories[i].Tasks[j].Completed {
					fmt.Printf("Task [%d] already completed.\n", taskID)
				} else {
					p.Categories[i].Tasks[j].Completed = true
					o.save()
					fmt.Printf("Task [%d] marked as completed.\n", taskID)
				}
				return
			}
		}
	}
	fmt.Printf("Task with ID %d not found in project '%s'.\n", taskID, projectName)
}

func (o *Organizer) progress(projectName string) {
	p := o.getProject(projectName)
	if p == nil {
		fmt.Printf("Project '%s' not found.\n", projectName)
		return
	}
	total, done := 0, 0
	for _, c := range p.Categories {
		for _, t := range c.Tasks {
			total++
			if t.Completed {
				done++
			}
		}
	}
	if total == 0 {
		fmt.Println("No tasks yet.")
		return
	}
	pct := float64(done) / float64(total) * 100
	fmt.Printf("\n📊 Progress for '%s': %d/%d tasks completed (%.1f%%)\n", p.Name, done, total, pct)
	barLen := 30
	filled := int(barLen * float64(done) / float64(total))
	bar := strings.Repeat("█", filled) + strings.Repeat("░", barLen-filled)
	fmt.Printf("[%s] %.1f%%\n", bar, pct)
}

func (o *Organizer) exportHTML(projectName string) {
	p := o.getProject(projectName)
	if p == nil {
		fmt.Printf("Project '%s' not found.\n", projectName)
		return
	}
	html := fmt.Sprintf(`<!DOCTYPE html>
<html><head><title>Checklist - %s</title>
<style>body{font-family:sans-serif;margin:30px;background:#f5f5f5;}
h1{color:#333;} .cat{background:#fff;border-radius:8px;padding:15px;margin:10px 0;box-shadow:0 2px 4px rgba(0,0,0,0.1);}
.task{padding:5px 0;border-bottom:1px solid #eee;}
.done{color:green;} .pending{color:red;}
</style></head><body>
<h1>📋 Checklist: %s</h1>
`, p.Name, p.Name)
	for _, c := range p.Categories {
		html += fmt.Sprintf("<div class='cat'><h2>📁 %s</h2>", c.Name)
		if len(c.Tasks) == 0 {
			html += "<p><em>No tasks</em></p>"
		} else {
			for _, t := range c.Tasks {
				cls := "done"
				mark := "✓"
				if !t.Completed {
					cls = "pending"
					mark = "✗"
				}
				html += fmt.Sprintf("<div class='task %s'><span>%s</span> %s</div>", cls, mark, t.Description)
			}
		}
		html += "</div>"
	}
	html += "</body></html>"
	filename := strings.ReplaceAll(p.Name, " ", "_") + "_checklist.html"
	os.WriteFile(filename, []byte(html), 0644)
	fmt.Printf("Report exported to %s\n", filename)
}

func main() {
	if len(os.Args) < 2 {
		fmt.Println("Usage: repair_checklist <command> [options]")
		return
	}
	app := NewOrganizer("projects.json")
	cmd := os.Args[1]
	switch cmd {
	case "create":
		if len(os.Args) < 3 {
			fmt.Println("create <name>")
			return
		}
		app.createProject(os.Args[2])
	case "add":
		if len(os.Args) < 5 {
			fmt.Println("add <project> <category> <task>")
			return
		}
		app.addTask(os.Args[2], os.Args[3], os.Args[4])
	case "list":
		if len(os.Args) < 3 {
			fmt.Println("list <project>")
			return
		}
		app.listTasks(os.Args[2])
	case "check":
		if len(os.Args) < 4 {
			fmt.Println("check <project> <task_id>")
			return
		}
		id, _ := strconv.Atoi(os.Args[3])
		app.checkTask(os.Args[2], id)
	case "progress":
		if len(os.Args) < 3 {
			fmt.Println("progress <project>")
			return
		}
		app.progress(os.Args[2])
	case "export":
		if len(os.Args) < 3 {
			fmt.Println("export <project>")
			return
		}
		app.exportHTML(os.Args[2])
	default:
		fmt.Println("Unknown command.")
	}
}
