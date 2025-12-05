shader_type spatial;
render_mode blend_mix, depth_draw_opaque, cull_back, diffuse_burley, specular_schlick_ggx;

uniform vec4 albedo_color : source_color = vec4(0.8, 0.3, 1.0, 1.0);
uniform float wobble_speed : hint_range(0.0, 10.0) = 2.0;
uniform float wobble_amplitude : hint_range(0.0, 1.0) = 0.1;
uniform float wobble_frequency : hint_range(1.0, 20.0) = 5.0;
uniform float metallic : hint_range(0.0, 1.0) = 0.5;
uniform float roughness : hint_range(0.0, 1.0) = 0.3;
uniform float emission_strength : hint_range(0.0, 3.0) = 0.5;

void vertex() {
    // Create wobble effect based on vertex position and time
    float wave1 = sin(VERTEX.y * wobble_frequency + TIME * wobble_speed);
    float wave2 = cos(VERTEX.x * wobble_frequency * 0.7 + TIME * wobble_speed * 1.3);
    float wave3 = sin(VERTEX.z * wobble_frequency * 0.5 + TIME * wobble_speed * 0.8);

    // Combine waves for organic movement
    vec3 offset = NORMAL * wobble_amplitude * (wave1 + wave2 + wave3) / 3.0;

    VERTEX += offset;
}

void fragment() {
    // Color variation based on normal for visual interest
    float color_var = dot(NORMAL, vec3(0.0, 1.0, 0.0)) * 0.5 + 0.5;
    vec3 final_color = mix(albedo_color.rgb, albedo_color.rgb * 1.5, color_var);

    ALBEDO = final_color;
    METALLIC = metallic;
    ROUGHNESS = roughness;
    EMISSION = albedo_color.rgb * emission_strength * (0.5 + color_var * 0.5);
}
