# minvr-images
An image viewer for VR environments using MinVR.

Input:

 A config file with this structure:

<images>
  <image-1>
    <file format="jpg">test.jpg</file>
    <shape>
      <type>rectangle</type>
      <location>x,y,z</location>
      <dimension>height,width</dimension>
    </shape>
  </image-1>
  <image-2>
    <file format="png">test2.png</file>
    <shape>
      <type>sphere</type>
      <location>x,y,z</location>
      <dimension>R</dimension>
    </shape>
  </image-2>
</images>