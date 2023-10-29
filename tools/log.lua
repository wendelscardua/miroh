str = ""

watch_table = {
  children = {}
}
current_watch = {}
label_stack = {}

function putchar_cb(address, value)
  c = string.char(value)
  if (c == '\n') then
    emu.log(str)
    str = ""
  else
    str = str .. c
    if (string.len(str) >= 80) then
      emu.log(str)
      str = ""
    end
  end
end

function start_watch(_address, label)
  emu.log('Start time ' .. label)
  current_watch = watch_table
  for k, v in ipairs(label_stack) do
    current_watch = current_watch.children[v]
  end
  if (current_watch.children[label] == nil) then
    current_watch.children[label] = {
      label = label,
      start = 0,
      cycles = 0,
      events = 0,
      frames = 0,
      children = {}
    }
  end
  current_watch = current_watch.children[label]
  current_watch.start = emu.getState()['cpu.cycleCount']
  table.insert(label_stack, label)
end

function stop_watch(_address, _value)
  current_watch = watch_table
  for k, v in ipairs(label_stack) do
    current_watch = current_watch.children[v]
  end

  label = table.remove(label_stack)
  emu.log('Stop time ' .. label)
  current_watch.events = current_watch.events + 1
  current_watch.cycles = current_watch.cycles + emu.getState()['cpu.cycleCount'] - current_watch.start
end

function recursive_display(subtable, level)
  if subtable.cycles ~= nil then
    emu.log("> " .. level .. " " .. subtable.label .. " " .. subtable.cycles .. " " .. subtable.events .. " " .. (subtable.cycles / subtable.events))
  end
  for label, inner in pairs(subtable.children) do
    recursive_display(inner, level + 1)
  end
end

function display_times()
  recursive_display(watch_table, 0)
end

emu.addMemoryCallback(putchar_cb, emu.callbackType.write, 0x4018)
emu.addMemoryCallback(start_watch, emu.callbackType.write, 0x4020)
emu.addMemoryCallback(stop_watch, emu.callbackType.write, 0x4021)
emu.addEventCallback(display_times, emu.eventType.endFrame);