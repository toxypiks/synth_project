#version  330

in vec2 fragTexCoord;
in vec4 fragColor;

out vec4 finalColor;
uniform vec4 colorParam;

void main()
{
  float x = fragTexCoord.x;
  float y = fragTexCoord.y;

  if (x < 0.5 && y > 0.120) {
    float color_factor = x*0.7;
    finalColor = color_factor * colorParam;
    finalColor.w = 1.0;
  }
  else if (x > 0.5 && y > 0.120) {
    float color_factor = (1 - x)*0.7;
    finalColor = color_factor * colorParam;
    finalColor.w = 1.0;
  }
  else if (y > 0.120) {
    finalColor = vec4(0, 0, 0, 0);
  }
}
