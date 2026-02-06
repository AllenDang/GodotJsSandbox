shader_type spatial;
render_mode blend_add, depth_draw_opaque, cull_disabled, unshaded;

uniform vec4 shield_color : source_color = vec4(0.2, 0.8, 1.0, 1.0);
uniform vec4 hex_color : source_color = vec4(0.0, 0.5, 1.0, 1.0);
uniform float hex_scale : hint_range(1.0, 20.0) = 8.0;
uniform float scroll_speed : hint_range(0.0, 5.0) = 0.5;
uniform float fresnel_power : hint_range(0.5, 10.0) = 2.0;
uniform float hex_intensity : hint_range(0.0, 1.0) = 0.3;
uniform float ripple_speed : hint_range(0.0, 10.0) = 3.0;
uniform float ripple_strength : hint_range(0.0, 1.0) = 0.2;
uniform float base_alpha : hint_range(0.0, 1.0) = 0.15;

// Hexagon pattern
float hex_distance(vec2 p) {
    p = abs(p);
    return max(p.x * 0.866025 + p.y * 0.5, p.y);
}

float hex_pattern(vec2 uv) {
    vec2 r = vec2(1.0, 1.732);
    vec2 h = r * 0.5;
    vec2 a = mod(uv, r) - h;
    vec2 b = mod(uv - h, r) - h;
    vec2 gv = length(a) < length(b) ? a : b;
    float d = hex_distance(gv);
    return smoothstep(0.4, 0.5, d);
}

void fragment() {
    // Fresnel for edge glow
    float fresnel = pow(1.0 - abs(dot(NORMAL, VIEW)), fresnel_power);

    // Scrolling hex pattern
    vec2 hex_uv = UV * hex_scale;
    hex_uv.y += TIME * scroll_speed;
    float hex = hex_pattern(hex_uv) * hex_intensity;

    // Ripple effect from center
    float dist = length(UV - vec2(0.5));
    float ripple = sin(dist * 20.0 - TIME * ripple_speed) * 0.5 + 0.5;
    ripple = ripple * ripple_strength * (1.0 - dist);

    // Combine effects
    float alpha = base_alpha + fresnel * 0.5 + hex * 0.2 + ripple;
    alpha = clamp(alpha, 0.0, 1.0);

    vec3 color = mix(shield_color.rgb, hex_color.rgb, hex);
    color += shield_color.rgb * ripple;
    color *= (1.0 + fresnel);

    ALBEDO = color;
    ALPHA = alpha;
}
