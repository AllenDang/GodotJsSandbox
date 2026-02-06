shader_type spatial;
render_mode blend_mix, depth_draw_opaque, cull_back, diffuse_burley, specular_schlick_ggx;

uniform vec4 albedo_color : source_color = vec4(0.1, 0.1, 0.2, 1.0);
uniform vec4 fresnel_color : source_color = vec4(0.3, 0.6, 1.0, 1.0);
uniform float fresnel_power : hint_range(0.5, 10.0) = 3.0;
uniform float emission_strength : hint_range(0.0, 5.0) = 2.0;
uniform float pulse_speed : hint_range(0.0, 5.0) = 1.0;
uniform float pulse_min : hint_range(0.0, 1.0) = 0.5;
uniform float metallic : hint_range(0.0, 1.0) = 0.8;
uniform float roughness : hint_range(0.0, 1.0) = 0.1;

void fragment() {
    // Fresnel effect
    float fresnel = pow(1.0 - abs(dot(NORMAL, VIEW)), fresnel_power);

    // Pulsing
    float pulse = mix(pulse_min, 1.0, (sin(TIME * pulse_speed * 3.14159) * 0.5 + 0.5));

    // Mix colors
    vec3 final_color = mix(albedo_color.rgb, fresnel_color.rgb, fresnel);

    ALBEDO = final_color;
    METALLIC = metallic;
    ROUGHNESS = roughness;
    EMISSION = fresnel_color.rgb * fresnel * emission_strength * pulse;
}
