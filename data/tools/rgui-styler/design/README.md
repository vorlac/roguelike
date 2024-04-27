<img align="left" src="../logo/rguistyler_128x128.png" width=128 height=128>

### rGuiStyler - tool evolution

*raygui_styler* tool development begin on mid-2015, [raygui](https://github.com/raysan5/raygui) project was started on December 2014 and after some time developing the library, a visual tool to customize style was required.

<br>

After some weeks of work, a first functional version of the visual style editor tool was released:

![raygui_styler - June 2015](./old/001_raygui_styler_july2015a.png)

Tool kept improving with some required additions like style import/export support and color palette saving.

![raygui_styler - January 2016](./old/003_raygui_styler_feb2016b.png)

Despite being a quite useful tool, it was never published and it was left aside for about a year. 

On **June 2016**, *raygui_styler* was picked up again to adapt to the new raygui improvements. At that moment lot of the tools internals get broken (despite visuals seem to work quite the same).

![raygui_styler - June 2016](./rguistyler2_light_REV0a.png)

The first big change was a brand new raygui color picker, generated in real-time, with one single draw call and more functional than the previous one (based on a loaded texture). It was by the end of 2017.

![raygui_styler - January 2017](./rguistyler2_light_REV0c.png)

**January 2017**, *raygui_styler*, renamed to **rGuiStyler** for raylib-based-tools naming consistency, suffered a big update. Previous list views for controls and properties were directly coded in the tool; with the new raygui list view control, they were not needed any more and were replaced by raygui based ones, tool layout was compacted and became more visual for the user.

![rGuiStyler - February 2017](./rguistyler2_light_REV4.png)

Tool UI/UX kept improving day after day, trying different options and refining details at the same time that raygui kept growing with new controls and styling options.

After some months of continuous review, here it is the final result: **rGuiStyler v2.0**

![rGuiStyler - April 2017](./rguistyler2_light_REV8.png)

*More screenshots about the tool design evolution could be found in this folder.*
