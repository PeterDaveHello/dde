class ShowDesktop extends AppItem
    is_fixed: true
    do_click: (e)->
        @show = false if not @show
        @show = !@show
        DCore.Dock.show_desktop(@show)
    do_buildmenu: ->
        []

class LauncherItem extends AppItem
    is_fixed: true
    do_click: (e)->
        @show = !@show
        DCore.run_command("launcher")
    do_buildmenu: ->
        []

show_desktop = new ShowDesktop("show_desktop", "img/desktop.png")
show_launcher = new LauncherItem("show_launcher", "img/launcher.png")
app_list.append(show_desktop)
app_list.append(show_launcher)