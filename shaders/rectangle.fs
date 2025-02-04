#version  330

in vec2 fragTexCoord;
in vec4 fragColor;

out vec4 finalColor;

void main()
{
  float x = fragTexCoord.x;
  float y = fragTexCoord.y;
  //finalColor = vec4(x, y, 0, 1);

  if (x < 0.5 && y > 0.120) {
    finalColor = vec4(0, x*0.7, 0, 1);
  }
  else if (x > 0.5 && y > 0.120) {
    finalColor = vec4(0, (1 - x)*0.7, 0, 1);
  }
  else if (y > 0.120) {
    finalColor = vec4(0, 0, 0, 0);
  }
}