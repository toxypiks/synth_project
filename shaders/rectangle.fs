#version  330

in vec2 fragTexCoord;
in vec4 fragColor;

out vec4 finalColor;

void main()
{
  float x = fragTexCoord.x;
  float y = fragTexCoord.y;
  //finalColor = vec4(x, y, 0, 1);

  if (x < 0.5) {
    finalColor = vec4(fragColor.xyz, x);
  }
  else {
  finalColor = vec4(fragColor.xyz, 1 - x);
  }
}