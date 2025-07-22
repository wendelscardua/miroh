str = ""

watch_table = {
  children = {}
}
current_watch = {}
label_stack = {}

display_toggle = false
left_mouse_prev_state = false

frame_start_cycle = 0

function get_start_frame_cycle_count()
  frame_start_cycle = emu.getState()['cpu.cycleCount']
end

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
  current_watch = watch_table
  for k, v in ipairs(label_stack) do
    current_watch = current_watch.children[v]
  end
  if (current_watch.children[label] == nil) then
    current_watch.children[label] = {
      label = label,
      start = 0,
      start_frame = 0,
      cycles = 0,
      children = {}
    }
  end
  current_watch = current_watch.children[label]
  current_watch.start = emu.getState()['cpu.cycleCount']
  current_watch.relative_start = current_watch.start - frame_start_cycle
  current_watch.start_frame = emu.getState()['frameCount']
  emu.getState()
  table.insert(label_stack, label)
end

function stop_watch(_address, label)
  current_watch = watch_table
  for k, v in ipairs(label_stack) do
    current_watch = current_watch.children[v]
  end

  removed_label = table.remove(label_stack)

  if removed_label ~= label then
    emu.log("Warning: closing label " .. label .. " doesn't match opening label " .. removed_label)
  end

  local new_cycles = emu.getState()['cpu.cycleCount'] - current_watch.start
  local frames = emu.getState()['frameCount'] - current_watch.start_frame
  if emu.getMouseState().right then
    current_watch.cycles = 0
  end
  if frames > 0 then
    emu.log("Warning: watch label " .. label .. " crossed " .. frames .. " frames (" .. new_cycles .. " cycles, starting at " .. current_watch.relative_start .. " cycles from beginning of its frame)")
    emu.log("Current frame: " .. emu.getState()['frameCount'])
  elseif new_cycles > current_watch.cycles then
    current_watch.cycles = new_cycles
  end
end

display_stack = {}

function recursive_display(subtable, x, y, width)
  local rect = { x = x, y = y, width = width, height = 2 }
  if subtable.cycles ~= nil then
    rect.label =  subtable.label .. " " .. string.format("%.2f", subtable.cycles)
    rect.height = rect.height + 10
  end
  for label, inner in pairs(subtable.children) do
    rect.height = rect.height + recursive_display(inner, x + 2, y + rect.height, width - 4) + 2
  end
  table.insert(display_stack, rect)
  return rect.height
end

function display_times()
  left_mouse_state = emu.getMouseState().left
  if left_mouse_state ~= left_mouse_prev_state then
    if left_mouse_state then
      display_toggle = not display_toggle
    end
    left_mouse_prev_state = left_mouse_state
  end

  if not display_toggle then
    return
  end

  display_stack = {}
  recursive_display(watch_table, 8, 8, 128)

  while #display_stack ~= 0 do
    rect = table.remove(display_stack)
    bgColor = 0x302060FF
    fgColor = 0x30FF4040

    emu.drawRectangle(rect.x, rect.y, rect.width, rect.height, bgColor, true, 1)
    emu.drawRectangle(rect.x, rect.y, rect.width, rect.height, fgColor, false, 1)
    if rect.label ~= nil then
      emu.drawString(rect.x + 2, rect.y + 2, rect.label, 0xFFFFFF, 0xFF000000)
    end
  end
end

function break_point(_address, value)
	emu.log("Breakpoint #" .. value)
	emu.breakExecution()
end

emu.addMemoryCallback(putchar_cb, emu.callbackType.write, 0x4018)
emu.addMemoryCallback(putchar_cb, emu.callbackType.write, 0x401b)
emu.addMemoryCallback(break_point, emu.callbackType.write, 0x4019)
emu.addMemoryCallback(start_watch, emu.callbackType.write, 0x4020)
emu.addMemoryCallback(stop_watch, emu.callbackType.write, 0x4021)
emu.addEventCallback(display_times, emu.eventType.endFrame);
emu.addEventCallback(get_start_frame_cycle_count, emu.eventType.startFrame);