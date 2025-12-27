# AbstractMuseum
An Unreal Engine plugin that simplifies creating virtual museums and exhibitions for Unreal users.
It allows creating BP instances from C++ classes inherited from ```AAbstractMuseumActor``` and placing them on the map.

# Main classes to save and customize data
```AAbstractMuseumActor```: Base class for all museum objects.

```AAbstractMuseumArt```: For art objects. Supports dynamic texture loading from .jpg or .png, 
auto scaling, and projection to the nearest wall (if ```bEnableProjection``` is enabled).
```AAbstractMuseumText```: For text objects (can be loaded from files or edited directly in the editor).
```AAbstractMuseumItem```: For 3D objects.

# Installation
 ```git clone ``` to Plugins folder

Built for Unreal Engine 5.5.4



