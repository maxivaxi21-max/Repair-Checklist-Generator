# repair_checklist.php
<?php
$dataFile = 'projects.json';

class Task {
    public $id, $description, $completed;
    function __construct($description, $completed = false, $id = 0) {
        $this->id = $id;
        $this->description = $description;
        $this->completed = $completed;
    }
}

class Category {
    public $name, $tasks = [];
    function __construct($name) { $this->name = $name; }
}

class Project {
    public $name, $categories = [];
    function __construct($name) { $this->name = $name; }
}

class Organizer {
    private $projects = [];
    private $file;

    function __construct($file) {
        $this->file = $file;
        $this->load();
    }

    function load() {
        if (file_exists($this->file)) {
            $data = json_decode(file_get_contents($this->file), true);
            foreach ($data as $p) {
                $proj = new Project($p['name']);
                foreach ($p['categories'] as $c) {
                    $cat = new Category($c['name']);
                    foreach ($c['tasks'] as $t) {
                        $task = new Task($t['description'], $t['completed'], $t['id']);
                        $cat->tasks[] = $task;
                    }
                    $proj->categories[] = $cat;
                }
                $this->projects[] = $proj;
            }
        }
    }

    function save() {
        $data = [];
        foreach ($this->projects as $p) {
            $proj = ['name' => $p->name, 'categories' => []];
            foreach ($p->categories as $c) {
                $cat = ['name' => $c->name, 'tasks' => []];
                foreach ($c->tasks as $t) {
                    $cat['tasks'][] = ['id' => $t->id, 'description' => $t->description, 'completed' => $t->completed];
                }
                $proj['categories'][] = $cat;
            }
            $data[] = $proj;
        }
        file_put_contents($this->file, json_encode($data, JSON_PRETTY_PRINT));
    }

    function getProject($name) {
        foreach ($this->projects as $p) {
            if ($p->name == $name) return $p;
        }
        return null;
    }

    function createProject($name) {
        if ($this->getProject($name)) {
            echo "Project '$name' already exists.\n";
            return;
        }
        $this->projects[] = new Project($name);
        $this->save();
        echo "Project '$name' created.\n";
    }

    function addTask($projectName, $categoryName, $taskDesc) {
        $p = $this->getProject($projectName);
        if (!$p) {
            echo "Project '$projectName' not found.\n";
            return;
        }
        $cat = null;
        foreach ($p->categories as $c) {
            if ($c->name == $categoryName) { $cat = $c; break; }
        }
        if (!$cat) {
            $cat = new Category($categoryName);
            $p->categories[] = $cat;
        }
        $maxId = 0;
        foreach ($p->categories as $c) {
            foreach ($c->tasks as $t) {
                if ($t->id > $maxId) $maxId = $t->id;
            }
        }
        $task = new Task($taskDesc, false, $maxId + 1);
        $cat->tasks[] = $task;
        $this->save();
        echo "Task added: [{$task->id}] $taskDesc (category: $categoryName)\n";
    }

    function listTasks($projectName) {
        $p = $this->getProject($projectName);
        if (!$p) {
            echo "Project '$projectName' not found.\n";
            return;
        }
        if (empty($p->categories)) {
            echo "No categories yet.\n";
            return;
        }
        echo "\n📋 Checklist for '{$p->name}':\n";
        foreach ($p->categories as $c) {
            echo "\n\033[36m📁 {$c->name}\033[0m\n";
            if (empty($c->tasks)) {
                echo "  (no tasks)\n";
            } else {
                foreach ($c->tasks as $t) {
                    $status = $t->completed ? "\033[32m✓\033[0m" : "\033[31m✗\033[0m";
                    echo "  [{$t->id}] $status {$t->description}\n";
                }
            }
        }
    }

    function checkTask($projectName, $taskId) {
        $p = $this->getProject($projectName);
        if (!$p) {
            echo "Project '$projectName' not found.\n";
            return;
        }
        foreach ($p->categories as $c) {
            foreach ($c->tasks as $t) {
                if ($t->id == $taskId) {
                    if ($t->completed) {
                        echo "Task [$taskId] already completed.\n";
                    } else {
                        $t->completed = true;
                        $this->save();
                        echo "Task [$taskId] marked as completed.\n";
                    }
                    return;
                }
            }
        }
        echo "Task with ID $taskId not found in project '$projectName'.\n";
    }

    function progress($projectName) {
        $p = $this->getProject($projectName);
        if (!$p) {
            echo "Project '$projectName' not found.\n";
            return;
        }
        $total = 0; $done = 0;
        foreach ($p->categories as $c) {
            foreach ($c->tasks as $t) {
                $total++;
                if ($t->completed) $done++;
            }
        }
        if ($total == 0) {
            echo "No tasks yet.\n";
            return;
        }
        $pct = ($done / $total) * 100;
        echo "\n📊 Progress for '{$p->name}': $done/$total tasks completed (" . number_format($pct,1) . "%)\n";
        $barLen = 30;
        $filled = (int)($barLen * $done / $total);
        $bar = str_repeat("█", $filled) . str_repeat("░", $barLen - $filled);
        echo "[$bar] " . number_format($pct,1) . "%\n";
    }

    function exportHTML($projectName) {
        $p = $this->getProject($projectName);
        if (!$p) {
            echo "Project '$projectName' not found.\n";
            return;
        }
        $html = "<!DOCTYPE html>
<html><head><title>Checklist - {$p->name}</title>
<style>body{font-family:sans-serif;margin:30px;background:#f5f5f5;}
h1{color:#333;} .cat{background:#fff;border-radius:8px;padding:15px;margin:10px 0;box-shadow:0 2px 4px rgba(0,0,0,0.1);}
.task{padding:5px 0;border-bottom:1px solid #eee;}
.done{color:green;} .pending{color:red;}
</style></head><body>
<h1>📋 Checklist: {$p->name}</h1>";
        foreach ($p->categories as $c) {
            $html .= "<div class='cat'><h2>📁 {$c->name}</h2>";
            if (empty($c->tasks)) {
                $html .= "<p><em>No tasks</em></p>";
            } else {
                foreach ($c->tasks as $t) {
                    $cls = $t->completed ? "done" : "pending";
                    $mark = $t->completed ? "✓" : "✗";
                    $html .= "<div class='task $cls'><span>$mark</span> {$t->description}</div>";
                }
            }
            $html .= "</div>";
        }
        $html .= "</body></html>";
        $filename = str_replace(' ', '_', $p->name) . '_checklist.html';
        file_put_contents($filename, $html);
        echo "Report exported to $filename\n";
    }
}

if ($argc < 2) {
    die("Usage: php repair_checklist.php <command> [options]\n");
}
$app = new Organizer($dataFile);
$cmd = $argv[1];

switch ($cmd) {
    case 'create':
        if ($argc < 3) die("create <name>\n");
        $app->createProject($argv[2]);
        break;
    case 'add':
        if ($argc < 5) die("add <project> <category> <task>\n");
        $app->addTask($argv[2], $argv[3], $argv[4]);
        break;
    case 'list':
        if ($argc < 3) die("list <project>\n");
        $app->listTasks($argv[2]);
        break;
    case 'check':
        if ($argc < 4) die("check <project> <task_id>\n");
        $app->checkTask($argv[2], (int)$argv[3]);
        break;
    case 'progress':
        if ($argc < 3) die("progress <project>\n");
        $app->progress($argv[2]);
        break;
    case 'export':
        if ($argc < 3) die("export <project>\n");
        $app->exportHTML($argv[2]);
        break;
    default:
        echo "Unknown command.\n";
}
?>
