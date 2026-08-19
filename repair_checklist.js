// repair_checklist.js
#!/usr/bin/env node
const fs = require('fs');
const path = require('path');
const { program } = require('commander');
const chalk = require('chalk');

const DATA_FILE = 'projects.json';

class Task {
    constructor(description, completed = false, id = 0) {
        this.id = id;
        this.description = description;
        this.completed = completed;
    }
}

class Category {
    constructor(name) {
        this.name = name;
        this.tasks = [];
    }
}

class Project {
    constructor(name) {
        this.name = name;
        this.categories = [];
    }
}

class Organizer {
    constructor() {
        this.projects = [];
        this.load();
    }

    load() {
        if (fs.existsSync(DATA_FILE)) {
            const data = JSON.parse(fs.readFileSync(DATA_FILE));
            this.projects = data.map(p => {
                const proj = new Project(p.name);
                proj.categories = p.categories.map(c => {
                    const cat = new Category(c.name);
                    cat.tasks = c.tasks.map(t => new Task(t.description, t.completed, t.id));
                    return cat;
                });
                return proj;
            });
        }
    }

    save() {
        const data = this.projects.map(p => ({
            name: p.name,
            categories: p.categories.map(c => ({
                name: c.name,
                tasks: c.tasks.map(t => ({ id: t.id, description: t.description, completed: t.completed }))
            }))
        }));
        fs.writeFileSync(DATA_FILE, JSON.stringify(data, null, 2));
    }

    getProject(name) {
        return this.projects.find(p => p.name === name);
    }

    createProject(name) {
        if (this.getProject(name)) {
            console.log(`Project '${name}' already exists.`);
            return;
        }
        this.projects.push(new Project(name));
        this.save();
        console.log(`Project '${name}' created.`);
    }

    addTask(projectName, categoryName, taskDesc) {
        const p = this.getProject(projectName);
        if (!p) {
            console.log(`Project '${projectName}' not found.`);
            return;
        }
        let cat = p.categories.find(c => c.name === categoryName);
        if (!cat) {
            cat = new Category(categoryName);
            p.categories.push(cat);
        }
        let maxId = 0;
        for (const c of p.categories) {
            for (const t of c.tasks) {
                if (t.id > maxId) maxId = t.id;
            }
        }
        const task = new Task(taskDesc, false, maxId + 1);
        cat.tasks.push(task);
        this.save();
        console.log(`Task added: [${task.id}] ${taskDesc} (category: ${categoryName})`);
    }

    listTasks(projectName) {
        const p = this.getProject(projectName);
        if (!p) {
            console.log(`Project '${projectName}' not found.`);
            return;
        }
        if (p.categories.length === 0) {
            console.log('No categories yet.');
            return;
        }
        console.log(`\n📋 Checklist for '${p.name}':`);
        for (const c of p.categories) {
            console.log(`\n${chalk.cyan(`📁 ${c.name}`)}`);
            if (c.tasks.length === 0) {
                console.log('  (no tasks)');
            } else {
                for (const t of c.tasks) {
                    const status = t.completed ? chalk.green('✓') : chalk.red('✗');
                    console.log(`  [${t.id}] ${status} ${t.description}`);
                }
            }
        }
    }

    checkTask(projectName, taskId) {
        const p = this.getProject(projectName);
        if (!p) {
            console.log(`Project '${projectName}' not found.`);
            return;
        }
        for (const c of p.categories) {
            for (const t of c.tasks) {
                if (t.id === taskId) {
                    if (t.completed) {
                        console.log(`Task [${taskId}] already completed.`);
                    } else {
                        t.completed = true;
                        this.save();
                        console.log(`Task [${taskId}] marked as completed.`);
                    }
                    return;
                }
            }
        }
        console.log(`Task with ID ${taskId} not found in project '${projectName}'.`);
    }

    progress(projectName) {
        const p = this.getProject(projectName);
        if (!p) {
            console.log(`Project '${projectName}' not found.`);
            return;
        }
        let total = 0, done = 0;
        for (const c of p.categories) {
            for (const t of c.tasks) {
                total++;
                if (t.completed) done++;
            }
        }
        if (total === 0) {
            console.log('No tasks yet.');
            return;
        }
        const pct = (done / total) * 100;
        console.log(`\n📊 Progress for '${p.name}': ${done}/${total} tasks completed (${pct.toFixed(1)}%)`);
        const barLen = 30;
        const filled = Math.round(barLen * done / total);
        const bar = '█'.repeat(filled) + '░'.repeat(barLen - filled);
        console.log(`[${bar}] ${pct.toFixed(1)}%`);
    }

    exportHTML(projectName) {
        const p = this.getProject(projectName);
        if (!p) {
            console.log(`Project '${projectName}' not found.`);
            return;
        }
        let html = `<!DOCTYPE html>
<html><head><title>Checklist - ${p.name}</title>
<style>body{font-family:sans-serif;margin:30px;background:#f5f5f5;}
h1{color:#333;} .cat{background:#fff;border-radius:8px;padding:15px;margin:10px 0;box-shadow:0 2px 4px rgba(0,0,0,0.1);}
.task{padding:5px 0;border-bottom:1px solid #eee;}
.done{color:green;} .pending{color:red;}
</style></head><body>
<h1>📋 Checklist: ${p.name}</h1>`;
        for (const c of p.categories) {
            html += `<div class='cat'><h2>📁 ${c.name}</h2>`;
            if (c.tasks.length === 0) {
                html += "<p><em>No tasks</em></p>";
            } else {
                for (const t of c.tasks) {
                    const cls = t.completed ? 'done' : 'pending';
                    const mark = t.completed ? '✓' : '✗';
                    html += `<div class='task ${cls}'><span>${mark}</span> ${t.description}</div>`;
                }
            }
            html += "</div>";
        }
        html += "</body></html>";
        const filename = p.name.replace(/ /g, '_') + '_checklist.html';
        fs.writeFileSync(filename, html);
        console.log(`Report exported to ${filename}`);
    }
}

program
    .command('create <name>')
    .action((name) => {
        const app = new Organizer();
        app.createProject(name);
    });

program
    .command('add <project> <category> <task>')
    .action((project, category, task) => {
        const app = new Organizer();
        app.addTask(project, category, task);
    });

program
    .command('list <project>')
    .action((project) => {
        const app = new Organizer();
        app.listTasks(project);
    });

program
    .command('check <project> <task_id>')
    .action((project, taskId) => {
        const app = new Organizer();
        app.checkTask(project, parseInt(taskId));
    });

program
    .command('progress <project>')
    .action((project) => {
        const app = new Organizer();
        app.progress(project);
    });

program
    .command('export <project>')
    .action((project) => {
        const app = new Organizer();
        app.exportHTML(project);
    });

program.parse(process.argv);
