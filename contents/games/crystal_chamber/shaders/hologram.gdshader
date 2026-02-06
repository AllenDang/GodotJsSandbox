shader_type spatial;
render_mode blend_add, depth_draw_opaque, cull_disabled, unshaded;

uniform vec4 hologram_color : source_color = vec4(0.0, 1.0, 0.8, 1.0);
uniform float scanline_speed : hint_range(0.0, 10.0) = 2.0;
uniform float scanline_count : hint_range(10.0, 200.0) = 50.0;
uniform float scanline_intensity : hint_range(0.0, 1.0) = 0.3;
uniform float flicker_speed : hint_range(0.0, 20.0) = 5.0;
uniform float flicker_intensity : hint_range(0.0, 1.0) = 0.1;
uniform float fresnel_power : hint_range(0.0, 10.0) = 3.0;
uniform float base_alpha : hint_range(0.0, 1.0) = 0.5;
uniform float glitch_intensity : hint_range(0.0, 1.0) = 0.2;

float random(float x) {
    return fract(sin(x * 12.9898) * 43758.5453);
}

void fragment() {
    // Fresnel effect (edge glow)
    float fresnel = pow(1.0 - abs(dot(NORMAL, VIEW)), fresnel_power);

    // Scanlines
    float scanline = sin((VERTEX.y + TIME * scanline_speed) * scanline_count) * 0.5 + 0.5;
    scanline = mix(1.0, scanline, scanline_intensity);

    // Flicker effect
    float flicker = 1.0 - flicker_intensity * random(floor(TIME * flicker_speed));

    // Glitch offset
    float glitch = step(0.99 - glitch_intensity * 0.1, random(floor(TIME * 10.0)));
    glitch *= random(floor(VERTEX.y * 20.0 + TIME * 5.0)) * glitch_intensity;

    // Combine effects
    float alpha = (base_alpha + fresnel * 0.5) * scanline * flicker;
    alpha = clamp(alpha, 0.0, 1.0);

    vec3 color = hologram_color.rgb * (1.0 + fresnel);

    // Add slight color shift for glitch
    color.r += glitch * 0.3;
    color.b -= glitch * 0.2;

    ALBEDO = color;
    ALPHA = alpha;
}
