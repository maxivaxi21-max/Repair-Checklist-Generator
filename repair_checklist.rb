# repair_checklist.rb
require 'json'
require 'colorize'
require 'optparse'

DATA_FILE = 'projects.json'

class Task
  attr_accessor :id, :description, :completed
  def initialize(description, completed = false, id = 0)
    @id = id
    @description = description
    @completed = completed
  end

  def to_hash
    { id: @id, description: @description, completed: @completed }
  end

  def self.from_hash(h)
    new(h['description'], h['completed'], h['id'])
  end
end

class Category
  attr_accessor :name, :tasks
  def initialize(name)
    @name = name
    @tasks = []
  end

  def to_hash
    { name: @name, tasks: @tasks.map(&:to_hash) }
  end

  def self.from_hash(h)
    c = new(h['name'])
    c.tasks = h['tasks'].map { |t| Task.from_hash(t) }
    c
  end
end

class Project
  attr_accessor :name, :categories
  def initialize(name)
    @name = name
    @categories = []
  end

  def to_hash
    { name: @name, categories: @categories.map(&:to_hash) }
  end

  def self.from_hash(h)
    p = new(h['name'])
    p.categories = h['categories'].map { |c| Category.from_hash(c) }
    p
  end
end

class Organizer
  attr_reader :projects

  def initialize
    @projects = []
    load
  end

  def load
    return unless File.exist?(DATA_FILE)
    data = JSON.parse(File.read(DATA_FILE))
    @projects = data.map { |p| Project.from_hash(p) }
  end

  def save
    File.write(DATA_FILE, JSON.pretty_generate(@projects.map(&:to_hash)))
  end

  def get_project(name)
    @projects.find { |p| p.name == name }
  end

  def create_project(name)
    if get_project(name)
      puts "Project '#{name}' already exists."
      return
    end
    @projects << Project.new(name)
    save
    puts "Project '#{name}' created."
  end

  def add_task(project_name, category_name, task_desc)
    p = get_project(project_name)
    unless p
      puts "Project '#{project_name}' not found."
      return
    end
    cat = p.categories.find { |c| c.name == category_name }
    unless cat
      cat = Category.new(category_name)
      p.categories << cat
    end
    max_id = p.categories.flat_map(&:tasks).map(&:id).max || 0
    task = Task.new(task_desc, false, max_id + 1)
    cat.tasks << task
    save
    puts "Task added: [#{task.id}] #{task_desc} (category: #{category_name})"
  end

  def list_tasks(project_name)
    p = get_project(project_name)
    unless p
      puts "Project '#{project_name}' not found."
      return
    end
    if p.categories.empty?
      puts "No categories yet."
      return
    end
    puts "\n📋 Checklist for '#{p.name}':"
    p.categories.each do |c|
      puts "\n📁 #{c.name}".cyan
      if c.tasks.empty?
        puts "  (no tasks)"
      else
        c.tasks.each do |t|
          status = t.completed ? "✓".green : "✗".red
          puts "  [#{t.id}] #{status} #{t.description}"
        end
      end
    end
  end

  def check_task(project_name, task_id)
    p = get_project(project_name)
    unless p
      puts "Project '#{project_name}' not found."
      return
    end
    p.categories.each do |c|
      c.tasks.each do |t|
        if t.id == task_id
          if t.completed
            puts "Task [#{task_id}] already completed."
          else
            t.completed = true
            save
            puts "Task [#{task_id}] marked as completed."
          end
          return
        end
      end
    end
    puts "Task with ID #{task_id} not found in project '#{project_name}'."
  end

  def progress(project_name)
    p = get_project(project_name)
    unless p
      puts "Project '#{project_name}' not found."
      return
    end
    total = p.categories.flat_map(&:tasks).size
    done = p.categories.flat_map(&:tasks).count(&:completed)
    if total == 0
      puts "No tasks yet."
      return
    end
    pct = (done.to_f / total) * 100
    puts "\n📊 Progress for '#{p.name}': #{done}/#{total} tasks completed (%.1f%%)" % pct
    bar_len = 30
    filled = (bar_len * done / total).to_i
    bar = "█" * filled + "░" * (bar_len - filled)
    puts "[#{bar}] %.1f%%" % pct
  end

  def export_html(project_name)
    p = get_project(project_name)
    unless p
      puts "Project '#{project_name}' not found."
      return
    end
    html = <<~HTML
    <!DOCTYPE html>
    <html><head><title>Checklist - #{p.name}</title>
    <style>body{font-family:sans-serif;margin:30px;background:#f5f5f5;}
    h1{color:#333;} .cat{background:#fff;border-radius:8px;padding:15px;margin:10px 0;box-shadow:0 2px 4px rgba(0,0,0,0.1);}
    .task{padding:5px 0;border-bottom:1px solid #eee;}
    .done{color:green;} .pending{color:red;}
    </style></head><body>
    <h1>📋 Checklist: #{p.name}</h1>
    HTML
    p.categories.each do |c|
      html += "<div class='cat'><h2>📁 #{c.name}</h2>"
      if c.tasks.empty?
        html += "<p><em>No tasks</em></p>"
      else
        c.tasks.each do |t|
          cls = t.completed ? "done" : "pending"
          mark = t.completed ? "✓" : "✗"
          html += "<div class='task #{cls}'><span>#{mark}</span> #{t.description}</div>"
        end
      end
      html += "</div>"
    end
    html += "</body></html>"
    filename = p.name.gsub(' ', '_') + '_checklist.html'
    File.write(filename, html)
    puts "Report exported to #{filename}"
  end
end

options = {}
$command = ARGV.shift
if $command.nil?
  puts "Usage: repair_checklist.rb <command> [options]"
  exit 1
end

app = Organizer.new

case $command
when "create"
  name = ARGV.shift
  app.create_project(name)
when "add"
  project, category, task = ARGV.shift(3)
  app.add_task(project, category, task)
when "list"
  project = ARGV.shift
  app.list_tasks(project)
when "check"
  project = ARGV.shift
  task_id = ARGV.shift.to_i
  app.check_task(project, task_id)
when "progress"
  project = ARGV.shift
  app.progress(project)
when "export"
  project = ARGV.shift
  app.export_html(project)
else
  puts "Unknown command."
end
