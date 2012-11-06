show = false
document.getElementById("icon_desktop").addEventListener('click', (e) ->
    show = !show
    DCore.Dock.show_desktop(show)
)
document.getElementById("icon_desktop").addEventListener('mouseover', (e) ->
    preview_close_window()
)

document.getElementById("icon_launcher").addEventListener('click', (e) ->
    DCore.run_command("launcher")
)
document.getElementById("icon_launcher").addEventListener('mouseover', (e) ->
    preview_close_window()
)

format_two_bit = (s) ->
    if s < 9
        return "0#{s}"
    else
        return s

get_time_str = ->
    today = new Date()
    hours = today.getHours()
    if hours > 12
        m = "PM"
        hours = hours - 12
    else
        m = "AM"
    hours = format_two_bit hours
    min = format_two_bit today.getMinutes()
    sec = format_two_bit today.getSeconds()
    n = DCore.Dock.test_get_n()
    return "#{hours}:#{min}:#{sec} #{m} #{n}"

c = document.getElementById("clock")
setInterval( ->
    c.innerText = get_time_str()
    return true
, 1000
)
