#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

out vec4 finalColor;

void main()
{
  float r = 0.5;

  // translation to center of texture
  vec2 p = fragTexCoord - vec2(0.5);

//  vec2 p_draw = fragTexCoord - vec2
  float s = length(p) - r;
  if (s <= -0.4){
    finalColor = fragColor;
  }
  else if ( s <= 0) {
    float color_value = (1.0 - 2.0*(s + 0.5)) * 0.35;
    finalColor = vec4(0, color_value, 0, 1);
  } else {
    finalColor = vec4(0, 0, 0, 1);
  }
}
