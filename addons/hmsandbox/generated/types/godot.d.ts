// Auto-generated. Do not edit.
/* eslint-disable @typescript-eslint/no-explicit-any */

import type { Vector2, Vector2i, Vector3, Vector3i, Vector4, Vector4i, Color, Quaternion, Basis, Transform3D } from "./math";
import type * as Enums from "./enums";

export type Signal = any;
export type Callable = (...args: any[]) => any;
export declare class RID { /* opaque */ }

export declare class PackedByteArray { __packed_handle: number; constructor(values?: any[]); }
export declare class PackedInt32Array { __packed_handle: number; constructor(values?: any[]); }
export declare class PackedInt64Array { __packed_handle: number; constructor(values?: any[]); }
export declare class PackedFloat32Array { __packed_handle: number; constructor(values?: any[]); }
export declare class PackedFloat64Array { __packed_handle: number; constructor(values?: any[]); }
export declare class PackedStringArray { __packed_handle: number; constructor(values?: any[]); }
export declare class PackedVector2Array { __packed_handle: number; constructor(values?: any[]); }
export declare class PackedVector3Array { __packed_handle: number; constructor(values?: any[]); }
export declare class PackedVector4Array { __packed_handle: number; constructor(values?: any[]); }
export declare class PackedColorArray { __packed_handle: number; constructor(values?: any[]); }

export declare class AESContext extends RefCounted {
  start(mode: number, key: PackedByteArray, iv?: PackedByteArray): number;
  update(src: PackedByteArray): PackedByteArray;
  get_iv_state(): PackedByteArray;
  finish(): void;
}

export declare class AStar2D extends RefCounted {
  neighbor_filter_enabled: boolean;
  get_available_point_id(): number;
  add_point(id: number, position: Vector2, weight_scale?: number): void;
  get_point_position(id: number): Vector2;
  set_point_position(id: number, position: Vector2): void;
  get_point_weight_scale(id: number): number;
  set_point_weight_scale(id: number, weight_scale: number): void;
  remove_point(id: number): void;
  has_point(id: number): boolean;
  get_point_connections(id: number): PackedInt64Array;
  get_point_ids(): PackedInt64Array;
  set_point_disabled(id: number, disabled?: boolean): void;
  is_point_disabled(id: number): boolean;
  connect_points(id: number, to_id: number, bidirectional?: boolean): void;
  disconnect_points(id: number, to_id: number, bidirectional?: boolean): void;
  are_points_connected(id: number, to_id: number, bidirectional?: boolean): boolean;
  get_point_count(): number;
  get_point_capacity(): number;
  reserve_space(num_nodes: number): void;
  clear(): void;
  get_closest_point(to_position: Vector2, include_disabled?: boolean): number;
  get_closest_position_in_segment(to_position: Vector2): Vector2;
  get_point_path(from_id: number, to_id: number, allow_partial_path?: boolean): PackedVector2Array;
  get_id_path(from_id: number, to_id: number, allow_partial_path?: boolean): PackedInt64Array;
}

export declare class AStar3D extends RefCounted {
  neighbor_filter_enabled: boolean;
  get_available_point_id(): number;
  add_point(id: number, position: Vector3, weight_scale?: number): void;
  get_point_position(id: number): Vector3;
  set_point_position(id: number, position: Vector3): void;
  get_point_weight_scale(id: number): number;
  set_point_weight_scale(id: number, weight_scale: number): void;
  remove_point(id: number): void;
  has_point(id: number): boolean;
  get_point_connections(id: number): PackedInt64Array;
  get_point_ids(): PackedInt64Array;
  set_point_disabled(id: number, disabled?: boolean): void;
  is_point_disabled(id: number): boolean;
  connect_points(id: number, to_id: number, bidirectional?: boolean): void;
  disconnect_points(id: number, to_id: number, bidirectional?: boolean): void;
  are_points_connected(id: number, to_id: number, bidirectional?: boolean): boolean;
  get_point_count(): number;
  get_point_capacity(): number;
  reserve_space(num_nodes: number): void;
  clear(): void;
  get_closest_point(to_position: Vector3, include_disabled?: boolean): number;
  get_closest_position_in_segment(to_position: Vector3): Vector3;
  get_point_path(from_id: number, to_id: number, allow_partial_path?: boolean): PackedVector3Array;
  get_id_path(from_id: number, to_id: number, allow_partial_path?: boolean): PackedInt64Array;
}

export declare class AStarGrid2D extends RefCounted {
  region: Rect2i;
  size: Vector2i;
  offset: Vector2;
  cell_size: Vector2;
  cell_shape: number;
  jumping_enabled: boolean;
  default_compute_heuristic: number;
  default_estimate_heuristic: number;
  diagonal_mode: number;
  is_in_bounds(x: number, y: number): boolean;
  is_in_boundsv(id: Vector2i): boolean;
  is_dirty(): boolean;
  update(): void;
  set_point_solid(id: Vector2i, solid?: boolean): void;
  is_point_solid(id: Vector2i): boolean;
  set_point_weight_scale(id: Vector2i, weight_scale: number): void;
  get_point_weight_scale(id: Vector2i): number;
  fill_solid_region(region: Rect2i, solid?: boolean): void;
  fill_weight_scale_region(region: Rect2i, weight_scale: number): void;
  clear(): void;
  get_point_position(id: Vector2i): Vector2;
  get_point_data_in_region(region: Rect2i): any[];
  get_point_path(from_id: Vector2i, to_id: Vector2i, allow_partial_path?: boolean): PackedVector2Array;
  get_id_path(from_id: Vector2i, to_id: Vector2i, allow_partial_path?: boolean): any[];
}

export declare class AcceptDialog extends Window {
  ok_button_text: string;
  dialog_text: string;
  dialog_hide_on_ok: boolean;
  dialog_close_on_escape: boolean;
  dialog_autowrap: boolean;
  get_ok_button(): Button;
  get_label(): Label;
  add_button(text: string, right?: boolean, action?: string): Button;
  add_cancel_button(name: string): Button;
  remove_button(button: Button): void;
  register_text_enter(line_edit: LineEdit): void;
}

export declare class AimModifier3D extends BoneConstraint3D {
  setting_count: number;
  set_forward_axis(index: number, axis: number): void;
  get_forward_axis(index: number): number;
  set_use_euler(index: number, enabled: boolean): void;
  is_using_euler(index: number): boolean;
  set_primary_rotation_axis(index: number, axis: number): void;
  get_primary_rotation_axis(index: number): number;
  set_use_secondary_rotation(index: number, enabled: boolean): void;
  is_using_secondary_rotation(index: number): boolean;
}

export declare class AnimatableBody2D extends StaticBody2D {
  sync_to_physics: boolean;
}

export declare class AnimatableBody3D extends StaticBody3D {
  sync_to_physics: boolean;
}

export declare class AnimatedSprite2D extends Node2D {
  sprite_frames: SpriteFrames;
  animation: string;
  frame: number;
  frame_progress: number;
  speed_scale: number;
  centered: boolean;
  offset: Vector2;
  flip_h: boolean;
  flip_v: boolean;
  is_playing(): boolean;
  play(name?: string, custom_speed?: number, from_end?: boolean): void;
  play_backwards(name?: string): void;
  pause(): void;
  stop(): void;
  set_frame_and_progress(frame: number, progress: number): void;
  get_playing_speed(): number;
}

export declare class AnimatedSprite3D extends SpriteBase3D {
  sprite_frames: SpriteFrames;
  animation: string;
  frame: number;
  frame_progress: number;
  speed_scale: number;
  is_playing(): boolean;
  play(name?: string, custom_speed?: number, from_end?: boolean): void;
  play_backwards(name?: string): void;
  pause(): void;
  stop(): void;
  set_frame_and_progress(frame: number, progress: number): void;
  get_playing_speed(): number;
}

export declare class AnimatedTexture extends Texture2D {
  frames: number;
  current_frame: number;
  pause: boolean;
  one_shot: boolean;
  speed_scale: number;
  set_frame_texture(frame: number, texture: Texture2D): void;
  get_frame_texture(frame: number): Texture2D;
  set_frame_duration(frame: number, duration: number): void;
  get_frame_duration(frame: number): number;
}

export declare class Animation extends Resource {
  length: number;
  loop_mode: number;
  step: number;
  readonly capture_included: boolean;
  add_track(type: number, at_position?: number): number;
  remove_track(track_idx: number): void;
  get_track_count(): number;
  track_get_type(track_idx: number): number;
  track_get_path(track_idx: number): string;
  track_set_path(track_idx: number, path: string): void;
  find_track(path: string, type: number): number;
  track_move_up(track_idx: number): void;
  track_move_down(track_idx: number): void;
  track_move_to(track_idx: number, to_idx: number): void;
  track_swap(track_idx: number, with_idx: number): void;
  track_set_imported(track_idx: number, imported: boolean): void;
  track_is_imported(track_idx: number): boolean;
  track_set_enabled(track_idx: number, enabled: boolean): void;
  track_is_enabled(track_idx: number): boolean;
  position_track_insert_key(track_idx: number, time: number, position: Vector3): number;
  rotation_track_insert_key(track_idx: number, time: number, rotation: Quaternion): number;
  scale_track_insert_key(track_idx: number, time: number, scale: Vector3): number;
  blend_shape_track_insert_key(track_idx: number, time: number, amount: number): number;
  position_track_interpolate(track_idx: number, time_sec: number, backward?: boolean): Vector3;
  rotation_track_interpolate(track_idx: number, time_sec: number, backward?: boolean): Quaternion;
  scale_track_interpolate(track_idx: number, time_sec: number, backward?: boolean): Vector3;
  blend_shape_track_interpolate(track_idx: number, time_sec: number, backward?: boolean): number;
  track_insert_key(track_idx: number, time: number, key: any, transition?: number): number;
  track_remove_key(track_idx: number, key_idx: number): void;
  track_remove_key_at_time(track_idx: number, time: number): void;
  track_set_key_value(track_idx: number, key: number, value: any): void;
  track_set_key_transition(track_idx: number, key_idx: number, transition: number): void;
  track_set_key_time(track_idx: number, key_idx: number, time: number): void;
  track_get_key_transition(track_idx: number, key_idx: number): number;
  track_get_key_count(track_idx: number): number;
  track_get_key_value(track_idx: number, key_idx: number): any;
  track_get_key_time(track_idx: number, key_idx: number): number;
  track_find_key(track_idx: number, time: number, find_mode?: number, limit?: boolean, backward?: boolean): number;
  track_set_interpolation_type(track_idx: number, interpolation: number): void;
  track_get_interpolation_type(track_idx: number): number;
  track_set_interpolation_loop_wrap(track_idx: number, interpolation: boolean): void;
  track_get_interpolation_loop_wrap(track_idx: number): boolean;
  track_is_compressed(track_idx: number): boolean;
  value_track_set_update_mode(track_idx: number, mode: number): void;
  value_track_get_update_mode(track_idx: number): number;
  value_track_interpolate(track_idx: number, time_sec: number, backward?: boolean): any;
  method_track_get_name(track_idx: number, key_idx: number): string;
  method_track_get_params(track_idx: number, key_idx: number): any[];
  bezier_track_insert_key(track_idx: number, time: number, value: number, in_handle?: Vector2, out_handle?: Vector2): number;
  bezier_track_set_key_value(track_idx: number, key_idx: number, value: number): void;
  bezier_track_set_key_in_handle(track_idx: number, key_idx: number, in_handle: Vector2, balanced_value_time_ratio?: number): void;
  bezier_track_set_key_out_handle(track_idx: number, key_idx: number, out_handle: Vector2, balanced_value_time_ratio?: number): void;
  bezier_track_get_key_value(track_idx: number, key_idx: number): number;
  bezier_track_get_key_in_handle(track_idx: number, key_idx: number): Vector2;
  bezier_track_get_key_out_handle(track_idx: number, key_idx: number): Vector2;
  bezier_track_interpolate(track_idx: number, time: number): number;
  audio_track_insert_key(track_idx: number, time: number, stream: Resource, start_offset?: number, end_offset?: number): number;
  audio_track_set_key_stream(track_idx: number, key_idx: number, stream: Resource): void;
  audio_track_set_key_start_offset(track_idx: number, key_idx: number, offset: number): void;
  audio_track_set_key_end_offset(track_idx: number, key_idx: number, offset: number): void;
  audio_track_get_key_stream(track_idx: number, key_idx: number): Resource;
  audio_track_get_key_start_offset(track_idx: number, key_idx: number): number;
  audio_track_get_key_end_offset(track_idx: number, key_idx: number): number;
  audio_track_set_use_blend(track_idx: number, enable: boolean): void;
  audio_track_is_use_blend(track_idx: number): boolean;
  animation_track_insert_key(track_idx: number, time: number, animation: string): number;
  animation_track_set_key_animation(track_idx: number, key_idx: number, animation: string): void;
  animation_track_get_key_animation(track_idx: number, key_idx: number): string;
  add_marker(name: string, time: number): void;
  remove_marker(name: string): void;
  has_marker(name: string): boolean;
  get_marker_at_time(time: number): string;
  get_next_marker(time: number): string;
  get_prev_marker(time: number): string;
  get_marker_time(name: string): number;
  get_marker_names(): PackedStringArray;
  get_marker_color(name: string): Color;
  set_marker_color(name: string, color: Color): void;
  clear(): void;
  copy_track(track_idx: number, to_animation: Animation): void;
  optimize(allowed_velocity_err?: number, allowed_angular_err?: number, precision?: number): void;
  compress(page_size?: number, fps?: number, split_tolerance?: number): void;
}

export declare class AnimationLibrary extends Resource {
  add_animation(name: string, animation: Animation): number;
  remove_animation(name: string): void;
  rename_animation(name: string, newname: string): void;
  has_animation(name: string): boolean;
  get_animation(name: string): Animation;
  get_animation_list(): any[];
  get_animation_list_size(): number;
}

export declare class AnimationMixer extends Node {
  active: boolean;
  deterministic: boolean;
  reset_on_save: boolean;
  root_node: string;
  root_motion_track: string;
  root_motion_local: boolean;
  audio_max_polyphony: number;
  callback_mode_process: number;
  callback_mode_method: number;
  callback_mode_discrete: number;
  add_animation_library(name: string, library: AnimationLibrary): number;
  remove_animation_library(name: string): void;
  rename_animation_library(name: string, newname: string): void;
  has_animation_library(name: string): boolean;
  get_animation_library(name: string): AnimationLibrary;
  get_animation_library_list(): any[];
  has_animation(name: string): boolean;
  get_animation(name: string): Animation;
  get_animation_list(): PackedStringArray;
  get_root_motion_position(): Vector3;
  get_root_motion_rotation(): Quaternion;
  get_root_motion_scale(): Vector3;
  get_root_motion_position_accumulator(): Vector3;
  get_root_motion_rotation_accumulator(): Quaternion;
  get_root_motion_scale_accumulator(): Vector3;
  clear_caches(): void;
  advance(delta: number): void;
  capture(name: string, duration: number, trans_type?: number, ease_type?: number): void;
  find_animation(animation: Animation): string;
  find_animation_library(animation: Animation): string;
}

export declare class AnimationNode extends Resource {
  filter_enabled: boolean;
  add_input(name: string): boolean;
  remove_input(index: number): void;
  set_input_name(input: number, name: string): boolean;
  get_input_name(input: number): string;
  get_input_count(): number;
  find_input(name: string): number;
  set_filter_path(path: string, enable: boolean): void;
  is_path_filtered(path: string): boolean;
  get_processing_animation_tree_instance_id(): number;
  is_process_testing(): boolean;
  blend_animation(animation: string, time: number, delta: number, seeked: boolean, is_external_seeking: boolean, blend: number, looped_flag?: number): void;
  blend_node(name: string, node: AnimationNode, time: number, seek: boolean, is_external_seeking: boolean, blend: number, filter?: number, sync?: boolean, test_only?: boolean): number;
  blend_input(input_index: number, time: number, seek: boolean, is_external_seeking: boolean, blend: number, filter?: number, sync?: boolean, test_only?: boolean): number;
  set_parameter(name: string, value: any): void;
  get_parameter(name: string): any;
}

export declare class AnimationNodeAdd2 extends AnimationNodeSync {
}

export declare class AnimationNodeAdd3 extends AnimationNodeSync {
}

export declare class AnimationNodeAnimation extends AnimationRootNode {
  animation: string;
  play_mode: number;
  advance_on_start: boolean;
  use_custom_timeline: boolean;
  timeline_length: number;
  stretch_time_scale: boolean;
  start_offset: number;
  loop_mode: number;
}

export declare class AnimationNodeBlend2 extends AnimationNodeSync {
}

export declare class AnimationNodeBlend3 extends AnimationNodeSync {
}

export declare class AnimationNodeBlendSpace1D extends AnimationRootNode {
  min_space: number;
  max_space: number;
  snap: number;
  value_label: string;
  blend_mode: number;
  sync: boolean;
  add_blend_point(node: AnimationRootNode, pos: number, at_index?: number): void;
  set_blend_point_position(point: number, pos: number): void;
  get_blend_point_position(point: number): number;
  set_blend_point_node(point: number, node: AnimationRootNode): void;
  get_blend_point_node(point: number): AnimationRootNode;
  remove_blend_point(point: number): void;
  get_blend_point_count(): number;
}

export declare class AnimationNodeBlendSpace2D extends AnimationRootNode {
  auto_triangles: boolean;
  min_space: Vector2;
  max_space: Vector2;
  snap: Vector2;
  x_label: string;
  y_label: string;
  blend_mode: number;
  sync: boolean;
  add_blend_point(node: AnimationRootNode, pos: Vector2, at_index?: number): void;
  set_blend_point_position(point: number, pos: Vector2): void;
  get_blend_point_position(point: number): Vector2;
  set_blend_point_node(point: number, node: AnimationRootNode): void;
  get_blend_point_node(point: number): AnimationRootNode;
  remove_blend_point(point: number): void;
  get_blend_point_count(): number;
  add_triangle(x: number, y: number, z: number, at_index?: number): void;
  get_triangle_point(triangle: number, point: number): number;
  remove_triangle(triangle: number): void;
  get_triangle_count(): number;
}

export declare class AnimationNodeBlendTree extends AnimationRootNode {
  graph_offset: Vector2;
  add_node(name: string, node: AnimationNode, position?: Vector2): void;
  get_node(name: string): AnimationNode;
  remove_node(name: string): void;
  rename_node(name: string, new_name: string): void;
  has_node(name: string): boolean;
  connect_node(input_node: string, input_index: number, output_node: string): void;
  disconnect_node(input_node: string, input_index: number): void;
  get_node_list(): any[];
  set_node_position(name: string, position: Vector2): void;
  get_node_position(name: string): Vector2;
}

export declare class AnimationNodeExtension extends AnimationNode {
  is_looping(node_info: PackedFloat32Array): boolean;
  get_remaining_time(node_info: PackedFloat32Array, break_loop: boolean): number;
}

export declare class AnimationNodeOneShot extends AnimationNodeSync {
  mix_mode: number;
  fadein_time: number;
  fadein_curve: Curve;
  fadeout_time: number;
  fadeout_curve: Curve;
  break_loop_at_end: boolean;
  autorestart: boolean;
  autorestart_delay: number;
  autorestart_random_delay: number;
}

export declare class AnimationNodeOutput extends AnimationNode {
}

export declare class AnimationNodeStateMachine extends AnimationRootNode {
  state_machine_type: number;
  allow_transition_to_self: boolean;
  reset_ends: boolean;
  add_node(name: string, node: AnimationNode, position?: Vector2): void;
  replace_node(name: string, node: AnimationNode): void;
  get_node(name: string): AnimationNode;
  remove_node(name: string): void;
  rename_node(name: string, new_name: string): void;
  has_node(name: string): boolean;
  get_node_name(node: AnimationNode): string;
  get_node_list(): any[];
  set_node_position(name: string, position: Vector2): void;
  get_node_position(name: string): Vector2;
  has_transition(from: string, to: string): boolean;
  add_transition(from: string, to: string, transition: AnimationNodeStateMachineTransition): void;
  get_transition(idx: number): AnimationNodeStateMachineTransition;
  get_transition_from(idx: number): string;
  get_transition_to(idx: number): string;
  get_transition_count(): number;
  remove_transition_by_index(idx: number): void;
  remove_transition(from: string, to: string): void;
  set_graph_offset(offset: Vector2): void;
  get_graph_offset(): Vector2;
}

export declare class AnimationNodeStateMachinePlayback extends Resource {
  travel(to_node: string, reset_on_teleport?: boolean): void;
  start(node: string, reset?: boolean): void;
  next(): void;
  stop(): void;
  is_playing(): boolean;
  get_current_node(): string;
  get_current_play_position(): number;
  get_current_length(): number;
  get_fading_from_node(): string;
  get_travel_path(): any[];
}

export declare class AnimationNodeStateMachineTransition extends Resource {
  xfade_time: number;
  xfade_curve: Curve;
  break_loop_at_end: boolean;
  reset: boolean;
  priority: number;
  switch_mode: number;
  advance_mode: number;
  advance_condition: string;
  advance_expression: string;
}

export declare class AnimationNodeSub2 extends AnimationNodeSync {
}

export declare class AnimationNodeSync extends AnimationNode {
  sync: boolean;
}

export declare class AnimationNodeTimeScale extends AnimationNode {
}

export declare class AnimationNodeTimeSeek extends AnimationNode {
  explicit_elapse: boolean;
}

export declare class AnimationNodeTransition extends AnimationNodeSync {
  xfade_time: number;
  xfade_curve: Curve;
  allow_transition_to_self: boolean;
  input_count: number;
  set_input_as_auto_advance(input: number, enable: boolean): void;
  is_input_set_as_auto_advance(input: number): boolean;
  set_input_break_loop_at_end(input: number, enable: boolean): void;
  is_input_loop_broken_at_end(input: number): boolean;
  set_input_reset(input: number, enable: boolean): void;
  is_input_reset(input: number): boolean;
}

export declare class AnimationPlayer extends AnimationMixer {
  readonly current_animation_length: number;
  readonly current_animation_position: number;
  playback_auto_capture: boolean;
  playback_auto_capture_duration: number;
  playback_auto_capture_transition_type: number;
  playback_auto_capture_ease_type: number;
  playback_default_blend_time: number;
  speed_scale: number;
  movie_quit_on_finish: boolean;
  animation_set_next(animation_from: string, animation_to: string): void;
  animation_get_next(animation_from: string): string;
  set_blend_time(animation_from: string, animation_to: string, sec: number): void;
  get_blend_time(animation_from: string, animation_to: string): number;
  play(name?: string, custom_blend?: number, custom_speed?: number, from_end?: boolean): void;
  play_section_with_markers(name?: string, start_marker?: string, end_marker?: string, custom_blend?: number, custom_speed?: number, from_end?: boolean): void;
  play_section(name?: string, start_time?: number, end_time?: number, custom_blend?: number, custom_speed?: number, from_end?: boolean): void;
  play_backwards(name?: string, custom_blend?: number): void;
  play_section_with_markers_backwards(name?: string, start_marker?: string, end_marker?: string, custom_blend?: number): void;
  play_section_backwards(name?: string, start_time?: number, end_time?: number, custom_blend?: number): void;
  play_with_capture(name?: string, duration?: number, custom_blend?: number, custom_speed?: number, from_end?: boolean, trans_type?: number, ease_type?: number): void;
  pause(): void;
  stop(keep_state?: boolean): void;
  is_playing(): boolean;
  queue(name: string): void;
  get_queue(): PackedStringArray;
  clear_queue(): void;
  get_playing_speed(): number;
  set_section_with_markers(start_marker?: string, end_marker?: string): void;
  set_section(start_time?: number, end_time?: number): void;
  reset_section(): void;
  get_section_start_time(): number;
  get_section_end_time(): number;
  has_section(): boolean;
  seek(seconds: number, update?: boolean, update_only?: boolean): void;
  set_process_callback(mode: number): void;
  get_process_callback(): number;
  set_method_call_mode(mode: number): void;
  get_method_call_mode(): number;
  set_root(path: string): void;
  get_root(): string;
}

export declare class AnimationRootNode extends AnimationNode {
}

export declare class AnimationTree extends AnimationMixer {
  tree_root: AnimationRootNode;
  advance_expression_base_node: string;
  anim_player: string;
  set_process_callback(mode: number): void;
  get_process_callback(): number;
}

export declare class Area2D extends CollisionObject2D {
  monitoring: boolean;
  monitorable: boolean;
  priority: number;
  gravity_space_override: number;
  gravity_point: boolean;
  gravity_point_unit_distance: number;
  gravity_point_center: Vector2;
  gravity_direction: Vector2;
  gravity: number;
  linear_damp_space_override: number;
  linear_damp: number;
  angular_damp_space_override: number;
  angular_damp: number;
  audio_bus_override: boolean;
  audio_bus_name: string;
  get_overlapping_bodies(): any[];
  get_overlapping_areas(): any[];
  has_overlapping_bodies(): boolean;
  has_overlapping_areas(): boolean;
  overlaps_body(body: Node): boolean;
  overlaps_area(area: Node): boolean;
}

export declare class Area3D extends CollisionObject3D {
  monitoring: boolean;
  monitorable: boolean;
  priority: number;
  gravity_space_override: number;
  gravity_point: boolean;
  gravity_point_unit_distance: number;
  gravity_point_center: Vector3;
  gravity_direction: Vector3;
  gravity: number;
  linear_damp_space_override: number;
  linear_damp: number;
  angular_damp_space_override: number;
  angular_damp: number;
  wind_force_magnitude: number;
  wind_attenuation_factor: number;
  wind_source_path: string;
  audio_bus_override: boolean;
  audio_bus_name: string;
  reverb_bus_enabled: boolean;
  reverb_bus_name: string;
  reverb_bus_amount: number;
  reverb_bus_uniformity: number;
  get_overlapping_bodies(): any[];
  get_overlapping_areas(): any[];
  has_overlapping_bodies(): boolean;
  has_overlapping_areas(): boolean;
  overlaps_body(body: Node): boolean;
  overlaps_area(area: Node): boolean;
}

export declare class ArrayMesh extends Mesh {
  blend_shape_mode: number;
  custom_aabb: AABB;
  shadow_mesh: ArrayMesh;
  add_blend_shape(name: string): void;
  get_blend_shape_count(): number;
  get_blend_shape_name(index: number): string;
  set_blend_shape_name(index: number, name: string): void;
  clear_blend_shapes(): void;
  add_surface_from_arrays(primitive: number, arrays: any[], blend_shapes?: any[], lods?: Record<string, any>, flags?: number): void;
  clear_surfaces(): void;
  surface_remove(surf_idx: number): void;
  surface_update_vertex_region(surf_idx: number, offset: number, data: PackedByteArray): void;
  surface_update_attribute_region(surf_idx: number, offset: number, data: PackedByteArray): void;
  surface_update_skin_region(surf_idx: number, offset: number, data: PackedByteArray): void;
  surface_get_array_len(surf_idx: number): number;
  surface_get_array_index_len(surf_idx: number): number;
  surface_get_format(surf_idx: number): number;
  surface_get_primitive_type(surf_idx: number): number;
  surface_find_by_name(name: string): number;
  surface_set_name(surf_idx: number, name: string): void;
  surface_get_name(surf_idx: number): string;
  regen_normal_maps(): void;
  lightmap_unwrap(transform: Transform3D, texel_size: number): number;
}

export declare class ArrayOccluder3D extends Occluder3D {
  vertices: PackedVector3Array;
  indices: PackedInt32Array;
  set_arrays(vertices: PackedVector3Array, indices: PackedInt32Array): void;
}

export declare class AspectRatioContainer extends Container {
  ratio: number;
  stretch_mode: number;
  alignment_horizontal: number;
  alignment_vertical: number;
}

export declare class AtlasTexture extends Texture2D {
  atlas: Texture2D;
  region: Rect2;
  margin: Rect2;
  filter_clip: boolean;
}

export declare class AudioBusLayout extends Resource {
}

export declare class AudioEffect extends Resource {
}

export declare class AudioEffectAmplify extends AudioEffect {
  volume_db: number;
  volume_linear: number;
}

export declare class AudioEffectBandLimitFilter extends AudioEffectFilter {
}

export declare class AudioEffectBandPassFilter extends AudioEffectFilter {
}

export declare class AudioEffectCapture extends AudioEffect {
  buffer_length: number;
  can_get_buffer(frames: number): boolean;
  get_buffer(frames: number): PackedVector2Array;
  clear_buffer(): void;
  get_frames_available(): number;
  get_discarded_frames(): number;
  get_buffer_length_frames(): number;
  get_pushed_frames(): number;
}

export declare class AudioEffectChorus extends AudioEffect {
  voice_count: number;
  dry: number;
  wet: number;
  set_voice_delay_ms(voice_idx: number, delay_ms: number): void;
  get_voice_delay_ms(voice_idx: number): number;
  set_voice_rate_hz(voice_idx: number, rate_hz: number): void;
  get_voice_rate_hz(voice_idx: number): number;
  set_voice_depth_ms(voice_idx: number, depth_ms: number): void;
  get_voice_depth_ms(voice_idx: number): number;
  set_voice_level_db(voice_idx: number, level_db: number): void;
  get_voice_level_db(voice_idx: number): number;
  set_voice_cutoff_hz(voice_idx: number, cutoff_hz: number): void;
  get_voice_cutoff_hz(voice_idx: number): number;
  set_voice_pan(voice_idx: number, pan: number): void;
  get_voice_pan(voice_idx: number): number;
}

export declare class AudioEffectCompressor extends AudioEffect {
  threshold: number;
  ratio: number;
  gain: number;
  attack_us: number;
  release_ms: number;
  mix: number;
  sidechain: string;
}

export declare class AudioEffectDelay extends AudioEffect {
  dry: number;
  tap1_active: boolean;
  tap1_delay_ms: number;
  tap1_level_db: number;
  tap1_pan: number;
  tap2_active: boolean;
  tap2_delay_ms: number;
  tap2_level_db: number;
  tap2_pan: number;
  feedback_active: boolean;
  feedback_delay_ms: number;
  feedback_level_db: number;
  feedback_lowpass: number;
}

export declare class AudioEffectDistortion extends AudioEffect {
  mode: number;
  pre_gain: number;
  keep_hf_hz: number;
  drive: number;
  post_gain: number;
}

export declare class AudioEffectEQ extends AudioEffect {
  set_band_gain_db(band_idx: number, volume_db: number): void;
  get_band_gain_db(band_idx: number): number;
  get_band_count(): number;
}

export declare class AudioEffectEQ10 extends AudioEffectEQ {
}

export declare class AudioEffectEQ21 extends AudioEffectEQ {
}

export declare class AudioEffectEQ6 extends AudioEffectEQ {
}

export declare class AudioEffectFilter extends AudioEffect {
  cutoff_hz: number;
  resonance: number;
  gain: number;
  db: number;
}

export declare class AudioEffectHardLimiter extends AudioEffect {
  pre_gain_db: number;
  ceiling_db: number;
  release: number;
}

export declare class AudioEffectHighPassFilter extends AudioEffectFilter {
}

export declare class AudioEffectHighShelfFilter extends AudioEffectFilter {
}

export declare class AudioEffectInstance extends RefCounted {
}

export declare class AudioEffectLimiter extends AudioEffect {
  ceiling_db: number;
  threshold_db: number;
  soft_clip_db: number;
  soft_clip_ratio: number;
}

export declare class AudioEffectLowPassFilter extends AudioEffectFilter {
}

export declare class AudioEffectLowShelfFilter extends AudioEffectFilter {
}

export declare class AudioEffectNotchFilter extends AudioEffectFilter {
}

export declare class AudioEffectPanner extends AudioEffect {
  pan: number;
}

export declare class AudioEffectPhaser extends AudioEffect {
  range_min_hz: number;
  range_max_hz: number;
  rate_hz: number;
  feedback: number;
  depth: number;
}

export declare class AudioEffectPitchShift extends AudioEffect {
  pitch_scale: number;
  oversampling: number;
  fft_size: number;
}

export declare class AudioEffectRecord extends AudioEffect {
  format: number;
  set_recording_active(record: boolean): void;
  is_recording_active(): boolean;
  get_recording(): AudioStreamWAV;
}

export declare class AudioEffectReverb extends AudioEffect {
  predelay_msec: number;
  predelay_feedback: number;
  room_size: number;
  damping: number;
  spread: number;
  hipass: number;
  dry: number;
  wet: number;
}

export declare class AudioEffectSpectrumAnalyzer extends AudioEffect {
  buffer_length: number;
  tap_back_pos: number;
  fft_size: number;
}

export declare class AudioEffectSpectrumAnalyzerInstance extends AudioEffectInstance {
  get_magnitude_for_frequency_range(from_hz: number, to_hz: number, mode?: number): Vector2;
}

export declare class AudioEffectStereoEnhance extends AudioEffect {
  pan_pullout: number;
  time_pullout_ms: number;
  surround: number;
}

export declare class AudioListener2D extends Node2D {
  make_current(): void;
  clear_current(): void;
  is_current(): boolean;
}

export declare class AudioListener3D extends Node3D {
  doppler_tracking: number;
  make_current(): void;
  clear_current(): void;
  is_current(): boolean;
  get_listener_transform(): Transform3D;
}

export declare class AudioSample extends RefCounted {
}

export declare class AudioSamplePlayback extends RefCounted {
}

export declare class AudioStream extends Resource {
  get_length(): number;
  is_monophonic(): boolean;
  instantiate_playback(): AudioStreamPlayback;
  can_be_sampled(): boolean;
  generate_sample(): AudioSample;
  is_meta_stream(): boolean;
}

export declare class AudioStreamGenerator extends AudioStream {
  mix_rate_mode: number;
  mix_rate: number;
  buffer_length: number;
}

export declare class AudioStreamGeneratorPlayback extends AudioStreamPlaybackResampled {
  push_frame(frame: Vector2): boolean;
  can_push_buffer(amount: number): boolean;
  push_buffer(frames: PackedVector2Array): boolean;
  get_frames_available(): number;
  get_skips(): number;
  clear_buffer(): void;
}

export declare class AudioStreamInteractive extends AudioStream {
  clip_count: number;
  initial_clip: number;
  set_clip_name(clip_index: number, name: string): void;
  get_clip_name(clip_index: number): string;
  set_clip_stream(clip_index: number, stream: AudioStream): void;
  get_clip_stream(clip_index: number): AudioStream;
  set_clip_auto_advance(clip_index: number, mode: number): void;
  get_clip_auto_advance(clip_index: number): number;
  set_clip_auto_advance_next_clip(clip_index: number, auto_advance_next_clip: number): void;
  get_clip_auto_advance_next_clip(clip_index: number): number;
  add_transition(from_clip: number, to_clip: number, from_time: number, to_time: number, fade_mode: number, fade_beats: number, use_filler_clip?: boolean, filler_clip?: number, hold_previous?: boolean): void;
  has_transition(from_clip: number, to_clip: number): boolean;
  erase_transition(from_clip: number, to_clip: number): void;
  get_transition_list(): PackedInt32Array;
  get_transition_from_time(from_clip: number, to_clip: number): number;
  get_transition_to_time(from_clip: number, to_clip: number): number;
  get_transition_fade_mode(from_clip: number, to_clip: number): number;
  get_transition_fade_beats(from_clip: number, to_clip: number): number;
  is_transition_using_filler_clip(from_clip: number, to_clip: number): boolean;
  get_transition_filler_clip(from_clip: number, to_clip: number): number;
  is_transition_holding_previous(from_clip: number, to_clip: number): boolean;
}

export declare class AudioStreamMP3 extends AudioStream {
  data: PackedByteArray;
  bpm: number;
  beat_count: number;
  bar_beats: number;
  loop: boolean;
  loop_offset: number;
  load_from_buffer(stream_data: PackedByteArray): AudioStreamMP3;
  load_from_file(path: string): AudioStreamMP3;
}

export declare class AudioStreamMicrophone extends AudioStream {
}

export declare class AudioStreamOggVorbis extends AudioStream {
  bpm: number;
  beat_count: number;
  bar_beats: number;
  tags: Record<string, any>;
  loop: boolean;
  loop_offset: number;
  load_from_buffer(stream_data: PackedByteArray): AudioStreamOggVorbis;
  load_from_file(path: string): AudioStreamOggVorbis;
}

export declare class AudioStreamPlayback extends RefCounted {
  set_sample_playback(playback_sample: AudioSamplePlayback): void;
  get_sample_playback(): AudioSamplePlayback;
  mix_audio(rate_scale: number, frames: number): PackedVector2Array;
  start(from_pos?: number): void;
  seek(time?: number): void;
  stop(): void;
  get_loop_count(): number;
  get_playback_position(): number;
  is_playing(): boolean;
}

export declare class AudioStreamPlaybackInteractive extends AudioStreamPlayback {
  switch_to_clip_by_name(clip_name: string): void;
  switch_to_clip(clip_index: number): void;
  get_current_clip_index(): number;
}

export declare class AudioStreamPlaybackOggVorbis extends AudioStreamPlaybackResampled {
}

export declare class AudioStreamPlaybackPlaylist extends AudioStreamPlayback {
}

export declare class AudioStreamPlaybackPolyphonic extends AudioStreamPlayback {
  play_stream(stream: AudioStream, from_offset?: number, volume_db?: number, pitch_scale?: number, playback_type?: number, bus?: string): number;
  set_stream_volume(stream: number, volume_db: number): void;
  set_stream_pitch_scale(stream: number, pitch_scale: number): void;
  is_stream_playing(stream: number): boolean;
  stop_stream(stream: number): void;
}

export declare class AudioStreamPlaybackResampled extends AudioStreamPlayback {
  begin_resample(): void;
}

export declare class AudioStreamPlaybackSynchronized extends AudioStreamPlayback {
}

export declare class AudioStreamPlayer extends Node {
  stream: AudioStream;
  volume_db: number;
  volume_linear: number;
  pitch_scale: number;
  playing: boolean;
  autoplay: boolean;
  stream_paused: boolean;
  mix_target: number;
  max_polyphony: number;
  bus: string;
  playback_type: number;
  play(from_position?: number): void;
  seek(to_position: number): void;
  stop(): void;
  get_playback_position(): number;
  has_stream_playback(): boolean;
  get_stream_playback(): AudioStreamPlayback;
}

export declare class AudioStreamPlayer2D extends Node2D {
  stream: AudioStream;
  volume_db: number;
  volume_linear: number;
  pitch_scale: number;
  playing: boolean;
  autoplay: boolean;
  stream_paused: boolean;
  max_distance: number;
  attenuation: number;
  max_polyphony: number;
  panning_strength: number;
  bus: string;
  area_mask: number;
  playback_type: number;
  play(from_position?: number): void;
  seek(to_position: number): void;
  stop(): void;
  get_playback_position(): number;
  has_stream_playback(): boolean;
  get_stream_playback(): AudioStreamPlayback;
}

export declare class AudioStreamPlayer3D extends Node3D {
  stream: AudioStream;
  attenuation_model: number;
  volume_db: number;
  volume_linear: number;
  unit_size: number;
  max_db: number;
  pitch_scale: number;
  playing: boolean;
  autoplay: boolean;
  stream_paused: boolean;
  max_distance: number;
  max_polyphony: number;
  panning_strength: number;
  bus: string;
  area_mask: number;
  playback_type: number;
  emission_angle_enabled: boolean;
  emission_angle_degrees: number;
  emission_angle_filter_attenuation_db: number;
  attenuation_filter_cutoff_hz: number;
  attenuation_filter_db: number;
  doppler_tracking: number;
  play(from_position?: number): void;
  seek(to_position: number): void;
  stop(): void;
  get_playback_position(): number;
  has_stream_playback(): boolean;
  get_stream_playback(): AudioStreamPlayback;
}

export declare class AudioStreamPlaylist extends AudioStream {
  shuffle: boolean;
  loop: boolean;
  fade_time: number;
  stream_count: number;
  get_bpm(): number;
}

export declare class AudioStreamPolyphonic extends AudioStream {
  polyphony: number;
}

export declare class AudioStreamRandomizer extends AudioStream {
  playback_mode: number;
  random_pitch: number;
  random_volume_offset_db: number;
  streams_count: number;
  add_stream(index: number, stream: AudioStream, weight?: number): void;
  move_stream(index_from: number, index_to: number): void;
  remove_stream(index: number): void;
  set_stream(index: number, stream: AudioStream): void;
  get_stream(index: number): AudioStream;
  set_stream_probability_weight(index: number, weight: number): void;
  get_stream_probability_weight(index: number): number;
}

export declare class AudioStreamSynchronized extends AudioStream {
  stream_count: number;
  set_sync_stream(stream_index: number, audio_stream: AudioStream): void;
  get_sync_stream(stream_index: number): AudioStream;
  set_sync_stream_volume(stream_index: number, volume_db: number): void;
  get_sync_stream_volume(stream_index: number): number;
}

export declare class AudioStreamWAV extends AudioStream {
  data: PackedByteArray;
  format: number;
  loop_mode: number;
  loop_begin: number;
  loop_end: number;
  mix_rate: number;
  stereo: boolean;
  tags: Record<string, any>;
  load_from_buffer(stream_data: PackedByteArray, options?: Record<string, any>): AudioStreamWAV;
  load_from_file(path: string, options?: Record<string, any>): AudioStreamWAV;
  save_to_wav(path: string): number;
}

export declare class BackBufferCopy extends Node2D {
  copy_mode: number;
  rect: Rect2;
}

export declare class BaseButton extends Control {
  disabled: boolean;
  toggle_mode: boolean;
  button_pressed: boolean;
  action_mode: number;
  button_mask: number;
  keep_pressed_outside: boolean;
  button_group: ButtonGroup;
  shortcut: Shortcut;
  shortcut_feedback: boolean;
  shortcut_in_tooltip: boolean;
  set_pressed_no_signal(pressed: boolean): void;
  is_hovered(): boolean;
  get_draw_mode(): number;
}

export declare class BaseMaterial3D extends Material {
  transparency: number;
  alpha_scissor_threshold: number;
  alpha_hash_scale: number;
  alpha_antialiasing_mode: number;
  alpha_antialiasing_edge: number;
  blend_mode: number;
  cull_mode: number;
  depth_draw_mode: number;
  depth_test: number;
  shading_mode: number;
  diffuse_mode: number;
  specular_mode: number;
  albedo_color: Color;
  metallic: number;
  metallic_specular: number;
  metallic_texture_channel: number;
  roughness: number;
  roughness_texture_channel: number;
  emission: Color;
  emission_energy_multiplier: number;
  emission_intensity: number;
  emission_operator: number;
  normal_scale: number;
  rim: number;
  rim_tint: number;
  clearcoat: number;
  clearcoat_roughness: number;
  anisotropy: number;
  ao_light_affect: number;
  ao_texture_channel: number;
  heightmap_scale: number;
  heightmap_deep_parallax: boolean;
  heightmap_min_layers: number;
  heightmap_max_layers: number;
  heightmap_flip_tangent: boolean;
  heightmap_flip_binormal: boolean;
  subsurf_scatter_strength: number;
  subsurf_scatter_transmittance_color: Color;
  subsurf_scatter_transmittance_depth: number;
  subsurf_scatter_transmittance_boost: number;
  backlight: Color;
  refraction_scale: number;
  refraction_texture_channel: number;
  detail_blend_mode: number;
  detail_uv_layer: number;
  uv1_scale: Vector3;
  uv1_offset: Vector3;
  uv1_triplanar_sharpness: number;
  uv2_scale: Vector3;
  uv2_offset: Vector3;
  uv2_triplanar_sharpness: number;
  texture_filter: number;
  billboard_mode: number;
  particles_anim_h_frames: number;
  particles_anim_v_frames: number;
  particles_anim_loop: boolean;
  grow: boolean;
  grow_amount: number;
  point_size: number;
  z_clip_scale: number;
  fov_override: number;
  proximity_fade_enabled: boolean;
  proximity_fade_distance: number;
  msdf_pixel_range: number;
  msdf_outline_size: number;
  distance_fade_mode: number;
  distance_fade_min_distance: number;
  distance_fade_max_distance: number;
  stencil_mode: number;
  stencil_flags: number;
  stencil_compare: number;
  stencil_reference: number;
  stencil_color: Color;
  stencil_outline_thickness: number;
}

export declare class BitMap extends Resource {
  create(size: Vector2i): void;
  create_from_image_alpha(image: Image, threshold?: number): void;
  set_bitv(position: Vector2i, bit: boolean): void;
  set_bit(x: number, y: number, bit: boolean): void;
  get_bitv(position: Vector2i): boolean;
  get_bit(x: number, y: number): boolean;
  set_bit_rect(rect: Rect2i, bit: boolean): void;
  get_true_bit_count(): number;
  get_size(): Vector2i;
  resize(new_size: Vector2i): void;
  grow_mask(pixels: number, rect: Rect2i): void;
  convert_to_image(): Image;
  opaque_to_polygons(rect: Rect2i, epsilon?: number): any[];
}

export declare class Bone2D extends Node2D {
  rest: Transform2D;
  apply_rest(): void;
  get_skeleton_rest(): Transform2D;
  get_index_in_skeleton(): number;
  set_autocalculate_length_and_angle(auto_calculate: boolean): void;
  get_autocalculate_length_and_angle(): boolean;
  set_length(length: number): void;
  get_length(): number;
  set_bone_angle(angle: number): void;
  get_bone_angle(): number;
}

export declare class BoneAttachment3D extends Node3D {
  bone_idx: number;
  override_pose: boolean;
  use_external_skeleton: boolean;
  external_skeleton: string;
  get_skeleton(): Skeleton3D;
  on_skeleton_update(): void;
}

export declare class BoneConstraint3D extends SkeletonModifier3D {
  set_amount(index: number, amount: number): void;
  get_amount(index: number): number;
  set_apply_bone_name(index: number, bone_name: string): void;
  get_apply_bone_name(index: number): string;
  set_apply_bone(index: number, bone: number): void;
  get_apply_bone(index: number): number;
  set_reference_bone_name(index: number, bone_name: string): void;
  get_reference_bone_name(index: number): string;
  set_reference_bone(index: number, bone: number): void;
  get_reference_bone(index: number): number;
  set_setting_count(count: number): void;
  get_setting_count(): number;
  clear_setting(): void;
}

export declare class BoneMap extends Resource {
  profile: SkeletonProfile;
  get_skeleton_bone_name(profile_bone_name: string): string;
  set_skeleton_bone_name(profile_bone_name: string, skeleton_bone_name: string): void;
  find_profile_bone_name(skeleton_bone_name: string): string;
}

export declare class BoxContainer extends Container {
  alignment: number;
  vertical: boolean;
  add_spacer(begin: boolean): Control;
}

export declare class BoxMesh extends PrimitiveMesh {
  size: Vector3;
  subdivide_width: number;
  subdivide_height: number;
  subdivide_depth: number;
}

export declare class BoxOccluder3D extends Occluder3D {
  size: Vector3;
}

export declare class BoxShape3D extends Shape3D {
  size: Vector3;
}

export declare class Button extends BaseButton {
  text: string;
  icon: Texture2D;
  flat: boolean;
  alignment: number;
  text_overrun_behavior: number;
  autowrap_mode: number;
  autowrap_trim_flags: number;
  clip_text: boolean;
  icon_alignment: number;
  vertical_icon_alignment: number;
  expand_icon: boolean;
  text_direction: number;
  language: string;
}

export declare class ButtonGroup extends Resource {
  allow_unpress: boolean;
  get_pressed_button(): BaseButton;
  get_buttons(): any[];
}

export declare class CPUParticles2D extends Node2D {
  emitting: boolean;
  amount: number;
  texture: Texture2D;
  lifetime: number;
  one_shot: boolean;
  preprocess: number;
  speed_scale: number;
  explosiveness: number;
  randomness: number;
  use_fixed_seed: boolean;
  seed: number;
  lifetime_randomness: number;
  fixed_fps: number;
  fract_delta: boolean;
  local_coords: boolean;
  draw_order: number;
  emission_shape: number;
  emission_sphere_radius: number;
  emission_rect_extents: Vector2;
  emission_points: PackedVector2Array;
  emission_normals: PackedVector2Array;
  emission_colors: PackedColorArray;
  direction: Vector2;
  spread: number;
  gravity: Vector2;
  split_scale: boolean;
  scale_curve_x: Curve;
  scale_curve_y: Curve;
  color: Color;
  color_ramp: Gradient;
  color_initial_ramp: Gradient;
  request_particles_process(process_time: number): void;
  restart(keep_seed?: boolean): void;
  convert_from_particles(particles: Node): void;
}

export declare class CPUParticles3D extends GeometryInstance3D {
  emitting: boolean;
  amount: number;
  lifetime: number;
  one_shot: boolean;
  preprocess: number;
  speed_scale: number;
  explosiveness: number;
  randomness: number;
  use_fixed_seed: boolean;
  seed: number;
  lifetime_randomness: number;
  fixed_fps: number;
  fract_delta: boolean;
  visibility_aabb: AABB;
  local_coords: boolean;
  draw_order: number;
  mesh: Mesh;
  emission_shape: number;
  emission_sphere_radius: number;
  emission_box_extents: Vector3;
  emission_points: PackedVector3Array;
  emission_normals: PackedVector3Array;
  emission_colors: PackedColorArray;
  emission_ring_axis: Vector3;
  emission_ring_height: number;
  emission_ring_radius: number;
  emission_ring_inner_radius: number;
  emission_ring_cone_angle: number;
  direction: Vector3;
  spread: number;
  flatness: number;
  gravity: Vector3;
  split_scale: boolean;
  scale_curve_x: Curve;
  scale_curve_y: Curve;
  scale_curve_z: Curve;
  color: Color;
  color_ramp: Gradient;
  color_initial_ramp: Gradient;
  restart(keep_seed?: boolean): void;
  request_particles_process(process_time: number): void;
  capture_aabb(): AABB;
  convert_from_particles(particles: Node): void;
}

export declare class CSGBox3D extends CSGPrimitive3D {
  size: Vector3;
}

export declare class CSGCombiner3D extends CSGShape3D {
}

export declare class CSGCylinder3D extends CSGPrimitive3D {
  radius: number;
  height: number;
  sides: number;
  cone: boolean;
  smooth_faces: boolean;
}

export declare class CSGMesh3D extends CSGPrimitive3D {
}

export declare class CSGPolygon3D extends CSGPrimitive3D {
  polygon: PackedVector2Array;
  mode: number;
  depth: number;
  spin_degrees: number;
  spin_sides: number;
  path_node: string;
  path_interval_type: number;
  path_interval: number;
  path_simplify_angle: number;
  path_rotation: number;
  path_rotation_accurate: boolean;
  path_local: boolean;
  path_continuous_u: boolean;
  path_u_distance: number;
  path_joined: boolean;
  smooth_faces: boolean;
}

export declare class CSGPrimitive3D extends CSGShape3D {
  flip_faces: boolean;
}

export declare class CSGShape3D extends GeometryInstance3D {
  operation: number;
  snap: number;
  calculate_tangents: boolean;
  use_collision: boolean;
  collision_layer: number;
  collision_mask: number;
  collision_priority: number;
  is_root_shape(): boolean;
  set_collision_mask_value(layer_number: number, value: boolean): void;
  get_collision_mask_value(layer_number: number): boolean;
  set_collision_layer_value(layer_number: number, value: boolean): void;
  get_collision_layer_value(layer_number: number): boolean;
  bake_collision_shape(): ConcavePolygonShape3D;
  get_meshes(): any[];
  bake_static_mesh(): ArrayMesh;
}

export declare class CSGSphere3D extends CSGPrimitive3D {
  radius: number;
  radial_segments: number;
  rings: number;
  smooth_faces: boolean;
}

export declare class CSGTorus3D extends CSGPrimitive3D {
  inner_radius: number;
  outer_radius: number;
  sides: number;
  ring_sides: number;
  smooth_faces: boolean;
}

export declare class CallbackTweener extends Tweener {
  set_delay(delay: number): CallbackTweener;
}

export declare class Camera2D extends Node2D {
  offset: Vector2;
  anchor_mode: number;
  ignore_rotation: boolean;
  enabled: boolean;
  zoom: Vector2;
  process_callback: number;
  limit_enabled: boolean;
  limit_smoothed: boolean;
  position_smoothing_enabled: boolean;
  position_smoothing_speed: number;
  rotation_smoothing_enabled: boolean;
  rotation_smoothing_speed: number;
  drag_horizontal_enabled: boolean;
  drag_vertical_enabled: boolean;
  drag_horizontal_offset: number;
  drag_vertical_offset: number;
  editor_draw_screen: boolean;
  editor_draw_limits: boolean;
  editor_draw_drag_margin: boolean;
  make_current(): void;
  is_current(): boolean;
  get_target_position(): Vector2;
  get_screen_center_position(): Vector2;
  get_screen_rotation(): number;
  force_update_scroll(): void;
  reset_smoothing(): void;
  align(): void;
}

export declare class Camera3D extends Node3D {
  keep_aspect: number;
  cull_mask: number;
  environment: Environment;
  compositor: Compositor;
  h_offset: number;
  v_offset: number;
  doppler_tracking: number;
  projection: number;
  current: boolean;
  fov: number;
  size: number;
  frustum_offset: Vector2;
  near: number;
  far: number;
  project_ray_normal(screen_point: Vector2): Vector3;
  project_local_ray_normal(screen_point: Vector2): Vector3;
  project_ray_origin(screen_point: Vector2): Vector3;
  unproject_position(world_point: Vector3): Vector2;
  is_position_behind(world_point: Vector3): boolean;
  project_position(screen_point: Vector2, z_depth: number): Vector3;
  set_perspective(fov: number, z_near: number, z_far: number): void;
  set_orthogonal(size: number, z_near: number, z_far: number): void;
  set_frustum(size: number, offset: Vector2, z_near: number, z_far: number): void;
  make_current(): void;
  clear_current(enable_next?: boolean): void;
  get_camera_transform(): Transform3D;
  get_camera_projection(): Projection;
  get_frustum(): any[];
  is_position_in_frustum(world_point: Vector3): boolean;
  get_camera_rid(): RID;
  get_pyramid_shape_rid(): RID;
  set_cull_mask_value(layer_number: number, value: boolean): void;
  get_cull_mask_value(layer_number: number): boolean;
}

export declare class CameraAttributes extends Resource {
  exposure_sensitivity: number;
  exposure_multiplier: number;
  auto_exposure_enabled: boolean;
  auto_exposure_scale: number;
  auto_exposure_speed: number;
}

export declare class CameraAttributesPhysical extends CameraAttributes {
  frustum_focus_distance: number;
  frustum_focal_length: number;
  frustum_near: number;
  frustum_far: number;
  exposure_aperture: number;
  exposure_shutter_speed: number;
  auto_exposure_min_exposure_value: number;
  auto_exposure_max_exposure_value: number;
  get_fov(): number;
}

export declare class CameraAttributesPractical extends CameraAttributes {
  dof_blur_far_enabled: boolean;
  dof_blur_far_distance: number;
  dof_blur_far_transition: number;
  dof_blur_near_enabled: boolean;
  dof_blur_near_distance: number;
  dof_blur_near_transition: number;
  dof_blur_amount: number;
  auto_exposure_min_sensitivity: number;
  auto_exposure_max_sensitivity: number;
}

export declare class CameraFeed extends RefCounted {
  feed_is_active: boolean;
  feed_transform: Transform2D;
  readonly formats: any[];
  get_id(): number;
  get_name(): string;
  set_name(name: string): void;
  get_position(): number;
  set_position(position: number): void;
  set_rgb_image(rgb_image: Image): void;
  set_ycbcr_image(ycbcr_image: Image): void;
  set_external(width: number, height: number): void;
  get_texture_tex_id(feed_image_type: number): number;
  get_datatype(): number;
  set_format(index: number, parameters: Record<string, any>): boolean;
}

export declare class CameraTexture extends Texture2D {
  camera_feed_id: number;
  which_feed: number;
  camera_is_active: boolean;
}

export declare class CanvasGroup extends Node2D {
  fit_margin: number;
  clear_margin: number;
  use_mipmaps: boolean;
}

export declare class CanvasItem extends Node {
  visible: boolean;
  modulate: Color;
  self_modulate: Color;
  show_behind_parent: boolean;
  top_level: boolean;
  clip_children: number;
  light_mask: number;
  visibility_layer: number;
  z_index: number;
  z_as_relative: boolean;
  y_sort_enabled: boolean;
  texture_filter: number;
  texture_repeat: number;
  use_parent_material: boolean;
  get_canvas_item(): RID;
  is_visible_in_tree(): boolean;
  show(): void;
  hide(): void;
  queue_redraw(): void;
  move_to_front(): void;
  draw_line(from: Vector2, to: Vector2, color: Color, width?: number, antialiased?: boolean): void;
  draw_dashed_line(from: Vector2, to: Vector2, color: Color, width?: number, dash?: number, aligned?: boolean, antialiased?: boolean): void;
  draw_polyline(points: PackedVector2Array, color: Color, width?: number, antialiased?: boolean): void;
  draw_polyline_colors(points: PackedVector2Array, colors: PackedColorArray, width?: number, antialiased?: boolean): void;
  draw_arc(center: Vector2, radius: number, start_angle: number, end_angle: number, point_count: number, color: Color, width?: number, antialiased?: boolean): void;
  draw_multiline(points: PackedVector2Array, color: Color, width?: number, antialiased?: boolean): void;
  draw_multiline_colors(points: PackedVector2Array, colors: PackedColorArray, width?: number, antialiased?: boolean): void;
  draw_rect(rect: Rect2, color: Color, filled?: boolean, width?: number, antialiased?: boolean): void;
  draw_circle(position: Vector2, radius: number, color: Color, filled?: boolean, width?: number, antialiased?: boolean): void;
  draw_texture(texture: Texture2D, position: Vector2, modulate?: Color): void;
  draw_texture_rect(texture: Texture2D, rect: Rect2, tile: boolean, modulate?: Color, transpose?: boolean): void;
  draw_texture_rect_region(texture: Texture2D, rect: Rect2, src_rect: Rect2, modulate?: Color, transpose?: boolean, clip_uv?: boolean): void;
  draw_msdf_texture_rect_region(texture: Texture2D, rect: Rect2, src_rect: Rect2, modulate?: Color, outline?: number, pixel_range?: number, scale?: number): void;
  draw_lcd_texture_rect_region(texture: Texture2D, rect: Rect2, src_rect: Rect2, modulate?: Color): void;
  draw_style_box(style_box: StyleBox, rect: Rect2): void;
  draw_primitive(points: PackedVector2Array, colors: PackedColorArray, uvs: PackedVector2Array, texture?: Texture2D): void;
  draw_polygon(points: PackedVector2Array, colors: PackedColorArray, uvs?: PackedVector2Array, texture?: Texture2D): void;
  draw_colored_polygon(points: PackedVector2Array, color: Color, uvs?: PackedVector2Array, texture?: Texture2D): void;
  draw_string(font: Font, pos: Vector2, text: string, alignment?: number, width?: number, font_size?: number, modulate?: Color, justification_flags?: number, direction?: number, orientation?: number, oversampling?: number): void;
  draw_multiline_string(font: Font, pos: Vector2, text: string, alignment?: number, width?: number, font_size?: number, max_lines?: number, modulate?: Color, brk_flags?: number, justification_flags?: number, direction?: number, orientation?: number, oversampling?: number): void;
  draw_string_outline(font: Font, pos: Vector2, text: string, alignment?: number, width?: number, font_size?: number, size?: number, modulate?: Color, justification_flags?: number, direction?: number, orientation?: number, oversampling?: number): void;
  draw_multiline_string_outline(font: Font, pos: Vector2, text: string, alignment?: number, width?: number, font_size?: number, max_lines?: number, size?: number, modulate?: Color, brk_flags?: number, justification_flags?: number, direction?: number, orientation?: number, oversampling?: number): void;
  draw_char(font: Font, pos: Vector2, char: string, font_size?: number, modulate?: Color, oversampling?: number): void;
  draw_char_outline(font: Font, pos: Vector2, char: string, font_size?: number, size?: number, modulate?: Color, oversampling?: number): void;
  draw_mesh(mesh: Mesh, texture: Texture2D, transform?: Transform2D, modulate?: Color): void;
  draw_multimesh(multimesh: MultiMesh, texture: Texture2D): void;
  draw_set_transform(position: Vector2, rotation?: number, scale?: Vector2): void;
  draw_set_transform_matrix(xform: Transform2D): void;
  draw_animation_slice(animation_length: number, slice_begin: number, slice_end: number, offset?: number): void;
  draw_end_animation(): void;
  get_transform(): Transform2D;
  get_global_transform(): Transform2D;
  get_global_transform_with_canvas(): Transform2D;
  get_viewport_transform(): Transform2D;
  get_viewport_rect(): Rect2;
  get_canvas_transform(): Transform2D;
  get_screen_transform(): Transform2D;
  get_local_mouse_position(): Vector2;
  get_global_mouse_position(): Vector2;
  get_canvas(): RID;
  get_canvas_layer_node(): CanvasLayer;
  get_world_2d(): World2D;
  set_instance_shader_parameter(name: string, value: any): void;
  get_instance_shader_parameter(name: string): any;
  set_notify_local_transform(enable: boolean): void;
  is_local_transform_notification_enabled(): boolean;
  set_notify_transform(enable: boolean): void;
  is_transform_notification_enabled(): boolean;
  force_update_transform(): void;
  make_canvas_position_local(viewport_point: Vector2): Vector2;
  make_input_local(event: InputEvent): InputEvent;
  set_visibility_layer_bit(layer: number, enabled: boolean): void;
  get_visibility_layer_bit(layer: number): boolean;
}

export declare class CanvasItemMaterial extends Material {
  blend_mode: number;
  light_mode: number;
  particles_animation: boolean;
  particles_anim_h_frames: number;
  particles_anim_v_frames: number;
  particles_anim_loop: boolean;
}

export declare class CanvasLayer extends Node {
  layer: number;
  visible: boolean;
  offset: Vector2;
  rotation: number;
  scale: Vector2;
  transform: Transform2D;
  follow_viewport_enabled: boolean;
  follow_viewport_scale: number;
  show(): void;
  hide(): void;
  get_final_transform(): Transform2D;
  get_canvas(): RID;
}

export declare class CanvasModulate extends Node2D {
  color: Color;
}

export declare class CanvasTexture extends Texture2D {
  diffuse_texture: Texture2D;
  normal_texture: Texture2D;
  specular_texture: Texture2D;
  specular_color: Color;
  specular_shininess: number;
  texture_filter: number;
  texture_repeat: number;
}

export declare class CapsuleMesh extends PrimitiveMesh {
  radius: number;
  height: number;
  radial_segments: number;
  rings: number;
}

export declare class CapsuleShape2D extends Shape2D {
  radius: number;
  height: number;
  mid_height: number;
}

export declare class CapsuleShape3D extends Shape3D {
  radius: number;
  height: number;
  mid_height: number;
}

export declare class CenterContainer extends Container {
  use_top_left: boolean;
}

export declare class CharFXTransform extends RefCounted {
  transform: Transform2D;
  range: Vector2i;
  elapsed_time: number;
  visible: boolean;
  outline: boolean;
  offset: Vector2;
  color: Color;
  env: Record<string, any>;
  glyph_index: number;
  glyph_count: number;
  glyph_flags: number;
  relative_index: number;
  font: RID;
}

export declare class CharacterBody2D extends PhysicsBody2D {
  motion_mode: number;
  up_direction: Vector2;
  velocity: Vector2;
  slide_on_ceiling: boolean;
  max_slides: number;
  wall_min_slide_angle: number;
  floor_stop_on_slope: boolean;
  floor_constant_speed: boolean;
  floor_block_on_wall: boolean;
  floor_max_angle: number;
  floor_snap_length: number;
  platform_on_leave: number;
  platform_floor_layers: number;
  platform_wall_layers: number;
  safe_margin: number;
  move_and_slide(): boolean;
  apply_floor_snap(): void;
  is_on_floor(): boolean;
  is_on_floor_only(): boolean;
  is_on_ceiling(): boolean;
  is_on_ceiling_only(): boolean;
  is_on_wall(): boolean;
  is_on_wall_only(): boolean;
  get_floor_normal(): Vector2;
  get_wall_normal(): Vector2;
  get_last_motion(): Vector2;
  get_position_delta(): Vector2;
  get_real_velocity(): Vector2;
  get_floor_angle(up_direction?: Vector2): number;
  get_platform_velocity(): Vector2;
  get_slide_collision_count(): number;
  get_slide_collision(slide_idx: number): KinematicCollision2D;
  get_last_slide_collision(): KinematicCollision2D;
}

export declare class CharacterBody3D extends PhysicsBody3D {
  motion_mode: number;
  up_direction: Vector3;
  slide_on_ceiling: boolean;
  velocity: Vector3;
  max_slides: number;
  wall_min_slide_angle: number;
  floor_stop_on_slope: boolean;
  floor_constant_speed: boolean;
  floor_block_on_wall: boolean;
  floor_max_angle: number;
  floor_snap_length: number;
  platform_on_leave: number;
  platform_floor_layers: number;
  platform_wall_layers: number;
  safe_margin: number;
  move_and_slide(): boolean;
  apply_floor_snap(): void;
  is_on_floor(): boolean;
  is_on_floor_only(): boolean;
  is_on_ceiling(): boolean;
  is_on_ceiling_only(): boolean;
  is_on_wall(): boolean;
  is_on_wall_only(): boolean;
  get_floor_normal(): Vector3;
  get_wall_normal(): Vector3;
  get_last_motion(): Vector3;
  get_position_delta(): Vector3;
  get_real_velocity(): Vector3;
  get_floor_angle(up_direction?: Vector3): number;
  get_platform_velocity(): Vector3;
  get_platform_angular_velocity(): Vector3;
  get_slide_collision_count(): number;
  get_slide_collision(slide_idx: number): KinematicCollision3D;
  get_last_slide_collision(): KinematicCollision3D;
}

export declare class CheckBox extends Button {
}

export declare class CheckButton extends Button {
}

export declare class CircleShape2D extends Shape2D {
  radius: number;
}

export declare class CodeEdit extends TextEdit {
  symbol_lookup_on_click: boolean;
  symbol_tooltip_on_hover: boolean;
  line_folding: boolean;
  gutters_draw_breakpoints_gutter: boolean;
  gutters_draw_bookmarks: boolean;
  gutters_draw_executing_lines: boolean;
  gutters_draw_line_numbers: boolean;
  gutters_zero_pad_line_numbers: boolean;
  gutters_draw_fold_gutter: boolean;
  code_completion_enabled: boolean;
  indent_size: number;
  indent_use_spaces: boolean;
  indent_automatic: boolean;
  auto_brace_completion_enabled: boolean;
  auto_brace_completion_highlight_matching: boolean;
  auto_brace_completion_pairs: Record<string, any>;
  do_indent(): void;
  indent_lines(): void;
  unindent_lines(): void;
  convert_indent(from_line?: number, to_line?: number): void;
  add_auto_brace_completion_pair(start_key: string, end_key: string): void;
  has_auto_brace_completion_open_key(open_key: string): boolean;
  has_auto_brace_completion_close_key(close_key: string): boolean;
  get_auto_brace_completion_close_key(open_key: string): string;
  set_line_as_breakpoint(line: number, breakpointed: boolean): void;
  is_line_breakpointed(line: number): boolean;
  clear_breakpointed_lines(): void;
  get_breakpointed_lines(): PackedInt32Array;
  set_line_as_bookmarked(line: number, bookmarked: boolean): void;
  is_line_bookmarked(line: number): boolean;
  clear_bookmarked_lines(): void;
  get_bookmarked_lines(): PackedInt32Array;
  set_line_as_executing(line: number, executing: boolean): void;
  is_line_executing(line: number): boolean;
  clear_executing_lines(): void;
  get_executing_lines(): PackedInt32Array;
  can_fold_line(line: number): boolean;
  fold_line(line: number): void;
  unfold_line(line: number): void;
  fold_all_lines(): void;
  unfold_all_lines(): void;
  toggle_foldable_line(line: number): void;
  toggle_foldable_lines_at_carets(): void;
  is_line_folded(line: number): boolean;
  get_folded_lines(): any[];
  create_code_region(): void;
  get_code_region_start_tag(): string;
  get_code_region_end_tag(): string;
  set_code_region_tags(start?: string, end?: string): void;
  is_line_code_region_start(line: number): boolean;
  is_line_code_region_end(line: number): boolean;
  add_string_delimiter(start_key: string, end_key: string, line_only?: boolean): void;
  remove_string_delimiter(start_key: string): void;
  has_string_delimiter(start_key: string): boolean;
  clear_string_delimiters(): void;
  is_in_string(line: number, column?: number): number;
  add_comment_delimiter(start_key: string, end_key: string, line_only?: boolean): void;
  remove_comment_delimiter(start_key: string): void;
  has_comment_delimiter(start_key: string): boolean;
  clear_comment_delimiters(): void;
  is_in_comment(line: number, column?: number): number;
  get_delimiter_start_key(delimiter_index: number): string;
  get_delimiter_end_key(delimiter_index: number): string;
  get_delimiter_start_position(line: number, column: number): Vector2;
  get_delimiter_end_position(line: number, column: number): Vector2;
  set_code_hint(code_hint: string): void;
  set_code_hint_draw_below(draw_below: boolean): void;
  get_text_for_code_completion(): string;
  request_code_completion(force?: boolean): void;
  add_code_completion_option(type: number, display_text: string, insert_text: string, text_color?: Color, icon?: Resource, value?: any, location?: number): void;
  update_code_completion_options(force: boolean): void;
  get_code_completion_options(): any[];
  get_code_completion_option(index: number): Record<string, any>;
  get_code_completion_selected_index(): number;
  set_code_completion_selected_index(index: number): void;
  confirm_code_completion(replace?: boolean): void;
  cancel_code_completion(): void;
  get_text_for_symbol_lookup(): string;
  get_text_with_cursor_char(line: number, column: number): string;
  set_symbol_lookup_word_as_valid(valid: boolean): void;
  move_lines_up(): void;
  move_lines_down(): void;
  delete_lines(): void;
  duplicate_selection(): void;
  duplicate_lines(): void;
}

export declare class CodeHighlighter extends SyntaxHighlighter {
  number_color: Color;
  symbol_color: Color;
  function_color: Color;
  member_variable_color: Color;
  keyword_colors: Record<string, any>;
  member_keyword_colors: Record<string, any>;
  color_regions: Record<string, any>;
  add_keyword_color(keyword: string, color: Color): void;
  remove_keyword_color(keyword: string): void;
  has_keyword_color(keyword: string): boolean;
  get_keyword_color(keyword: string): Color;
  clear_keyword_colors(): void;
  add_member_keyword_color(member_keyword: string, color: Color): void;
  remove_member_keyword_color(member_keyword: string): void;
  has_member_keyword_color(member_keyword: string): boolean;
  get_member_keyword_color(member_keyword: string): Color;
  clear_member_keyword_colors(): void;
  add_color_region(start_key: string, end_key: string, color: Color, line_only?: boolean): void;
  remove_color_region(start_key: string): void;
  has_color_region(start_key: string): boolean;
  clear_color_regions(): void;
}

export declare class CollisionObject2D extends Node2D {
  disable_mode: number;
  collision_layer: number;
  collision_mask: number;
  collision_priority: number;
  input_pickable: boolean;
  get_rid(): RID;
  set_collision_layer_value(layer_number: number, value: boolean): void;
  get_collision_layer_value(layer_number: number): boolean;
  set_collision_mask_value(layer_number: number, value: boolean): void;
  get_collision_mask_value(layer_number: number): boolean;
  create_shape_owner(owner: any): number;
  remove_shape_owner(owner_id: number): void;
  get_shape_owners(): PackedInt32Array;
  shape_owner_set_transform(owner_id: number, transform: Transform2D): void;
  shape_owner_get_transform(owner_id: number): Transform2D;
  shape_owner_get_owner(owner_id: number): any;
  shape_owner_set_disabled(owner_id: number, disabled: boolean): void;
  is_shape_owner_disabled(owner_id: number): boolean;
  shape_owner_set_one_way_collision(owner_id: number, enable: boolean): void;
  is_shape_owner_one_way_collision_enabled(owner_id: number): boolean;
  shape_owner_set_one_way_collision_margin(owner_id: number, margin: number): void;
  get_shape_owner_one_way_collision_margin(owner_id: number): number;
  shape_owner_add_shape(owner_id: number, shape: Shape2D): void;
  shape_owner_get_shape_count(owner_id: number): number;
  shape_owner_get_shape(owner_id: number, shape_id: number): Shape2D;
  shape_owner_get_shape_index(owner_id: number, shape_id: number): number;
  shape_owner_remove_shape(owner_id: number, shape_id: number): void;
  shape_owner_clear_shapes(owner_id: number): void;
  shape_find_owner(shape_index: number): number;
}

export declare class CollisionObject3D extends Node3D {
  disable_mode: number;
  collision_layer: number;
  collision_mask: number;
  collision_priority: number;
  input_ray_pickable: boolean;
  input_capture_on_drag: boolean;
  set_collision_layer_value(layer_number: number, value: boolean): void;
  get_collision_layer_value(layer_number: number): boolean;
  set_collision_mask_value(layer_number: number, value: boolean): void;
  get_collision_mask_value(layer_number: number): boolean;
  get_rid(): RID;
  create_shape_owner(owner: any): number;
  remove_shape_owner(owner_id: number): void;
  get_shape_owners(): PackedInt32Array;
  shape_owner_set_transform(owner_id: number, transform: Transform3D): void;
  shape_owner_get_transform(owner_id: number): Transform3D;
  shape_owner_get_owner(owner_id: number): any;
  shape_owner_set_disabled(owner_id: number, disabled: boolean): void;
  is_shape_owner_disabled(owner_id: number): boolean;
  shape_owner_add_shape(owner_id: number, shape: Shape3D): void;
  shape_owner_get_shape_count(owner_id: number): number;
  shape_owner_get_shape(owner_id: number, shape_id: number): Shape3D;
  shape_owner_get_shape_index(owner_id: number, shape_id: number): number;
  shape_owner_remove_shape(owner_id: number, shape_id: number): void;
  shape_owner_clear_shapes(owner_id: number): void;
  shape_find_owner(shape_index: number): number;
}

export declare class CollisionPolygon2D extends Node2D {
  build_mode: number;
  polygon: PackedVector2Array;
  disabled: boolean;
  one_way_collision: boolean;
  one_way_collision_margin: number;
}

export declare class CollisionPolygon3D extends Node3D {
  depth: number;
  disabled: boolean;
  polygon: PackedVector2Array;
  margin: number;
  debug_color: Color;
  debug_fill: boolean;
}

export declare class CollisionShape2D extends Node2D {
  shape: Shape2D;
  disabled: boolean;
  one_way_collision: boolean;
  one_way_collision_margin: number;
  debug_color: Color;
}

export declare class CollisionShape3D extends Node3D {
  shape: Shape3D;
  disabled: boolean;
  debug_color: Color;
  debug_fill: boolean;
  resource_changed(resource: Resource): void;
  make_convex_from_siblings(): void;
}

export declare class ColorPalette extends Resource {
  colors: PackedColorArray;
}

export declare class ColorPicker extends VBoxContainer {
  color: Color;
  edit_alpha: boolean;
  edit_intensity: boolean;
  color_mode: number;
  deferred_mode: boolean;
  picker_shape: number;
  can_add_swatches: boolean;
  sampler_visible: boolean;
  color_modes_visible: boolean;
  sliders_visible: boolean;
  hex_visible: boolean;
  presets_visible: boolean;
  add_preset(color: Color): void;
  erase_preset(color: Color): void;
  get_presets(): PackedColorArray;
  add_recent_preset(color: Color): void;
  erase_recent_preset(color: Color): void;
  get_recent_presets(): PackedColorArray;
}

export declare class ColorPickerButton extends Button {
  color: Color;
  edit_alpha: boolean;
  edit_intensity: boolean;
  get_picker(): ColorPicker;
  get_popup(): PopupPanel;
}

export declare class ColorRect extends Control {
  color: Color;
}

export declare class Compositor extends Resource {
}

export declare class CompositorEffect extends Resource {
  enabled: boolean;
  effect_callback_type: number;
  access_resolved_color: boolean;
  access_resolved_depth: boolean;
  needs_motion_vectors: boolean;
  needs_normal_roughness: boolean;
  needs_separate_specular: boolean;
}

export declare class CompressedCubemap extends CompressedTextureLayered {
}

export declare class CompressedCubemapArray extends CompressedTextureLayered {
}

export declare class CompressedTexture2D extends Texture2D {
  load_path: string;
}

export declare class CompressedTexture2DArray extends CompressedTextureLayered {
}

export declare class CompressedTexture3D extends Texture3D {
  load_path: string;
}

export declare class CompressedTextureLayered extends TextureLayered {
  load_path: string;
}

export declare class ConcavePolygonShape2D extends Shape2D {
  segments: PackedVector2Array;
}

export declare class ConcavePolygonShape3D extends Shape3D {
  data: PackedVector3Array;
  backface_collision: boolean;
}

export declare class ConeTwistJoint3D extends Joint3D {
}

export declare class ConfigFile extends RefCounted {
  set_value(section: string, key: string, value: any): void;
  get_value(section: string, key: string, default?: any): any;
  has_section(section: string): boolean;
  has_section_key(section: string, key: string): boolean;
  get_sections(): PackedStringArray;
  get_section_keys(section: string): PackedStringArray;
  erase_section(section: string): void;
  erase_section_key(section: string, key: string): void;
  load(path: string): number;
  parse(data: string): number;
  save(path: string): number;
  encode_to_text(): string;
  load_encrypted(path: string, key: PackedByteArray): number;
  load_encrypted_pass(path: string, password: string): number;
  save_encrypted(path: string, key: PackedByteArray): number;
  save_encrypted_pass(path: string, password: string): number;
  clear(): void;
}

export declare class ConfirmationDialog extends AcceptDialog {
  cancel_button_text: string;
  get_cancel_button(): Button;
}

export declare class Container extends Control {
  queue_sort(): void;
  fit_child_in_rect(child: Control, rect: Rect2): void;
}

export declare class Control extends CanvasItem {
  clip_contents: boolean;
  custom_minimum_size: Vector2;
  layout_direction: number;
  grow_horizontal: number;
  grow_vertical: number;
  rotation: number;
  rotation_degrees: number;
  scale: Vector2;
  pivot_offset: Vector2;
  size_flags_horizontal: number;
  size_flags_vertical: number;
  size_flags_stretch_ratio: number;
  localize_numeral_system: boolean;
  auto_translate: boolean;
  tooltip_text: string;
  tooltip_auto_translate_mode: number;
  focus_next: string;
  focus_previous: string;
  focus_mode: number;
  focus_behavior_recursive: number;
  mouse_filter: number;
  mouse_behavior_recursive: number;
  mouse_force_pass_scroll_events: boolean;
  mouse_default_cursor_shape: number;
  accessibility_name: string;
  accessibility_description: string;
  accessibility_live: number;
  accessibility_controls_nodes: any[];
  accessibility_described_by_nodes: any[];
  accessibility_labeled_by_nodes: any[];
  accessibility_flow_to_nodes: any[];
  theme: Theme;
  accept_event(): void;
  get_minimum_size(): Vector2;
  get_combined_minimum_size(): Vector2;
  set_offsets_preset(preset: number, resize_mode?: number, margin?: number): void;
  set_anchors_and_offsets_preset(preset: number, resize_mode?: number, margin?: number): void;
  set_anchor(side: number, anchor: number, keep_offset?: boolean, push_opposite_anchor?: boolean): void;
  set_anchor_and_offset(side: number, anchor: number, offset: number, push_opposite_anchor?: boolean): void;
  set_begin(position: Vector2): void;
  set_end(position: Vector2): void;
  reset_size(): void;
  get_begin(): Vector2;
  get_end(): Vector2;
  get_parent_area_size(): Vector2;
  get_screen_position(): Vector2;
  get_rect(): Rect2;
  get_global_rect(): Rect2;
  get_focus_mode_with_override(): number;
  has_focus(): boolean;
  grab_focus(): void;
  release_focus(): void;
  find_prev_valid_focus(): Control;
  find_next_valid_focus(): Control;
  find_valid_focus_neighbor(side: number): Control;
  begin_bulk_theme_override(): void;
  end_bulk_theme_override(): void;
  add_theme_icon_override(name: string, texture: Texture2D): void;
  add_theme_stylebox_override(name: string, stylebox: StyleBox): void;
  add_theme_font_override(name: string, font: Font): void;
  add_theme_font_size_override(name: string, font_size: number): void;
  add_theme_color_override(name: string, color: Color): void;
  add_theme_constant_override(name: string, constant: number): void;
  remove_theme_icon_override(name: string): void;
  remove_theme_stylebox_override(name: string): void;
  remove_theme_font_override(name: string): void;
  remove_theme_font_size_override(name: string): void;
  remove_theme_color_override(name: string): void;
  remove_theme_constant_override(name: string): void;
  get_theme_icon(name: string, theme_type?: string): Texture2D;
  get_theme_stylebox(name: string, theme_type?: string): StyleBox;
  get_theme_font(name: string, theme_type?: string): Font;
  get_theme_font_size(name: string, theme_type?: string): number;
  get_theme_color(name: string, theme_type?: string): Color;
  get_theme_constant(name: string, theme_type?: string): number;
  has_theme_icon_override(name: string): boolean;
  has_theme_stylebox_override(name: string): boolean;
  has_theme_font_override(name: string): boolean;
  has_theme_font_size_override(name: string): boolean;
  has_theme_color_override(name: string): boolean;
  has_theme_constant_override(name: string): boolean;
  has_theme_icon(name: string, theme_type?: string): boolean;
  has_theme_stylebox(name: string, theme_type?: string): boolean;
  has_theme_font(name: string, theme_type?: string): boolean;
  has_theme_font_size(name: string, theme_type?: string): boolean;
  has_theme_color(name: string, theme_type?: string): boolean;
  has_theme_constant(name: string, theme_type?: string): boolean;
  get_theme_default_base_scale(): number;
  get_theme_default_font(): Font;
  get_theme_default_font_size(): number;
  get_parent_control(): Control;
  get_tooltip(at_position?: Vector2): string;
  get_cursor_shape(position?: Vector2): number;
  force_drag(data: any, preview: Control): void;
  accessibility_drag(): void;
  accessibility_drop(): void;
  get_mouse_filter_with_override(): number;
  grab_click_focus(): void;
  set_drag_forwarding(drag_func: Callable, can_drop_func: Callable, drop_func: Callable): void;
  set_drag_preview(control: Control): void;
  is_drag_successful(): boolean;
  warp_mouse(position: Vector2): void;
  update_minimum_size(): void;
  is_layout_rtl(): boolean;
}

export declare class ConvertTransformModifier3D extends BoneConstraint3D {
  setting_count: number;
  set_apply_transform_mode(index: number, transform_mode: number): void;
  get_apply_transform_mode(index: number): number;
  set_apply_axis(index: number, axis: number): void;
  get_apply_axis(index: number): number;
  set_apply_range_min(index: number, range_min: number): void;
  get_apply_range_min(index: number): number;
  set_apply_range_max(index: number, range_max: number): void;
  get_apply_range_max(index: number): number;
  set_reference_transform_mode(index: number, transform_mode: number): void;
  get_reference_transform_mode(index: number): number;
  set_reference_axis(index: number, axis: number): void;
  get_reference_axis(index: number): number;
  set_reference_range_min(index: number, range_min: number): void;
  get_reference_range_min(index: number): number;
  set_reference_range_max(index: number, range_max: number): void;
  get_reference_range_max(index: number): number;
  set_relative(index: number, enabled: boolean): void;
  is_relative(index: number): boolean;
  set_additive(index: number, enabled: boolean): void;
  is_additive(index: number): boolean;
}

export declare class ConvexPolygonShape2D extends Shape2D {
  points: PackedVector2Array;
  set_point_cloud(point_cloud: PackedVector2Array): void;
}

export declare class ConvexPolygonShape3D extends Shape3D {
}

export declare class CopyTransformModifier3D extends BoneConstraint3D {
  setting_count: number;
  set_copy_flags(index: number, copy_flags: number): void;
  get_copy_flags(index: number): number;
  set_axis_flags(index: number, axis_flags: number): void;
  get_axis_flags(index: number): number;
  set_invert_flags(index: number, axis_flags: number): void;
  get_invert_flags(index: number): number;
  set_copy_position(index: number, enabled: boolean): void;
  is_position_copying(index: number): boolean;
  set_copy_rotation(index: number, enabled: boolean): void;
  is_rotation_copying(index: number): boolean;
  set_copy_scale(index: number, enabled: boolean): void;
  is_scale_copying(index: number): boolean;
  set_axis_x_enabled(index: number, enabled: boolean): void;
  is_axis_x_enabled(index: number): boolean;
  set_axis_y_enabled(index: number, enabled: boolean): void;
  is_axis_y_enabled(index: number): boolean;
  set_axis_z_enabled(index: number, enabled: boolean): void;
  is_axis_z_enabled(index: number): boolean;
  set_axis_x_inverted(index: number, enabled: boolean): void;
  is_axis_x_inverted(index: number): boolean;
  set_axis_y_inverted(index: number, enabled: boolean): void;
  is_axis_y_inverted(index: number): boolean;
  set_axis_z_inverted(index: number, enabled: boolean): void;
  is_axis_z_inverted(index: number): boolean;
  set_relative(index: number, enabled: boolean): void;
  is_relative(index: number): boolean;
  set_additive(index: number, enabled: boolean): void;
  is_additive(index: number): boolean;
}

export declare class Crypto extends RefCounted {
  generate_random_bytes(size: number): PackedByteArray;
  generate_rsa(size: number): CryptoKey;
  generate_self_signed_certificate(key: CryptoKey, issuer_name?: string, not_before?: string, not_after?: string): X509Certificate;
  sign(hash_type: number, hash: PackedByteArray, key: CryptoKey): PackedByteArray;
  verify(hash_type: number, hash: PackedByteArray, signature: PackedByteArray, key: CryptoKey): boolean;
  encrypt(key: CryptoKey, plaintext: PackedByteArray): PackedByteArray;
  decrypt(key: CryptoKey, ciphertext: PackedByteArray): PackedByteArray;
  hmac_digest(hash_type: number, key: PackedByteArray, msg: PackedByteArray): PackedByteArray;
  constant_time_compare(trusted: PackedByteArray, received: PackedByteArray): boolean;
}

export declare class CryptoKey extends Resource {
  save(path: string, public_only?: boolean): number;
  load(path: string, public_only?: boolean): number;
  is_public_only(): boolean;
  save_to_string(public_only?: boolean): string;
  load_from_string(string_key: string, public_only?: boolean): number;
}

export declare class Cubemap extends ImageTextureLayered {
  create_placeholder(): Resource;
}

export declare class CubemapArray extends ImageTextureLayered {
  create_placeholder(): Resource;
}

export declare class Curve extends Resource {
  min_domain: number;
  max_domain: number;
  min_value: number;
  max_value: number;
  bake_resolution: number;
  point_count: number;
  add_point(position: Vector2, left_tangent?: number, right_tangent?: number, left_mode?: number, right_mode?: number): number;
  remove_point(index: number): void;
  clear_points(): void;
  get_point_position(index: number): Vector2;
  set_point_value(index: number, y: number): void;
  set_point_offset(index: number, offset: number): number;
  sample(offset: number): number;
  sample_baked(offset: number): number;
  get_point_left_tangent(index: number): number;
  get_point_right_tangent(index: number): number;
  get_point_left_mode(index: number): number;
  get_point_right_mode(index: number): number;
  set_point_left_tangent(index: number, tangent: number): void;
  set_point_right_tangent(index: number, tangent: number): void;
  set_point_left_mode(index: number, mode: number): void;
  set_point_right_mode(index: number, mode: number): void;
  get_value_range(): number;
  get_domain_range(): number;
  clean_dupes(): void;
  bake(): void;
}

export declare class Curve2D extends Resource {
  bake_interval: number;
  point_count: number;
  add_point(position: Vector2, in?: Vector2, out?: Vector2, index?: number): void;
  set_point_position(idx: number, position: Vector2): void;
  get_point_position(idx: number): Vector2;
  set_point_in(idx: number, position: Vector2): void;
  get_point_in(idx: number): Vector2;
  set_point_out(idx: number, position: Vector2): void;
  get_point_out(idx: number): Vector2;
  remove_point(idx: number): void;
  clear_points(): void;
  sample(idx: number, t: number): Vector2;
  samplef(fofs: number): Vector2;
  get_baked_length(): number;
  sample_baked(offset?: number, cubic?: boolean): Vector2;
  sample_baked_with_rotation(offset?: number, cubic?: boolean): Transform2D;
  get_baked_points(): PackedVector2Array;
  get_closest_point(to_point: Vector2): Vector2;
  get_closest_offset(to_point: Vector2): number;
  tessellate(max_stages?: number, tolerance_degrees?: number): PackedVector2Array;
  tessellate_even_length(max_stages?: number, tolerance_length?: number): PackedVector2Array;
}

export declare class Curve3D extends Resource {
  closed: boolean;
  bake_interval: number;
  point_count: number;
  up_vector_enabled: boolean;
  add_point(position: Vector3, in?: Vector3, out?: Vector3, index?: number): void;
  set_point_position(idx: number, position: Vector3): void;
  get_point_position(idx: number): Vector3;
  set_point_tilt(idx: number, tilt: number): void;
  get_point_tilt(idx: number): number;
  set_point_in(idx: number, position: Vector3): void;
  get_point_in(idx: number): Vector3;
  set_point_out(idx: number, position: Vector3): void;
  get_point_out(idx: number): Vector3;
  remove_point(idx: number): void;
  clear_points(): void;
  sample(idx: number, t: number): Vector3;
  samplef(fofs: number): Vector3;
  get_baked_length(): number;
  sample_baked(offset?: number, cubic?: boolean): Vector3;
  sample_baked_with_rotation(offset?: number, cubic?: boolean, apply_tilt?: boolean): Transform3D;
  sample_baked_up_vector(offset: number, apply_tilt?: boolean): Vector3;
  get_baked_points(): PackedVector3Array;
  get_baked_tilts(): PackedFloat32Array;
  get_baked_up_vectors(): PackedVector3Array;
  get_closest_point(to_point: Vector3): Vector3;
  get_closest_offset(to_point: Vector3): number;
  tessellate(max_stages?: number, tolerance_degrees?: number): PackedVector3Array;
  tessellate_even_length(max_stages?: number, tolerance_length?: number): PackedVector3Array;
}

export declare class CurveTexture extends Texture2D {
  width: number;
  texture_mode: number;
  curve: Curve;
}

export declare class CurveXYZTexture extends Texture2D {
  width: number;
  curve_x: Curve;
  curve_y: Curve;
  curve_z: Curve;
}

export declare class CylinderMesh extends PrimitiveMesh {
  top_radius: number;
  bottom_radius: number;
  height: number;
  radial_segments: number;
  rings: number;
  cap_top: boolean;
  cap_bottom: boolean;
}

export declare class CylinderShape3D extends Shape3D {
  height: number;
  radius: number;
}

export declare class DPITexture extends Texture2D {
  base_scale: number;
  saturation: number;
  create_from_string(source: string, scale?: number, saturation?: number, color_map?: Record<string, any>): DPITexture;
  set_source(source: string): void;
  get_source(): string;
  set_size_override(size: Vector2i): void;
  get_scaled_rid(): RID;
}

export declare class DampedSpringJoint2D extends Joint2D {
  length: number;
  rest_length: number;
  stiffness: number;
  damping: number;
}

export declare class Decal extends VisualInstance3D {
  size: Vector3;
  emission_energy: number;
  modulate: Color;
  albedo_mix: number;
  normal_fade: number;
  upper_fade: number;
  lower_fade: number;
  distance_fade_enabled: boolean;
  distance_fade_begin: number;
  distance_fade_length: number;
  cull_mask: number;
}

export declare class DirectionalLight2D extends Light2D {
  height: number;
  max_distance: number;
}

export declare class DirectionalLight3D extends Light3D {
  directional_shadow_mode: number;
  directional_shadow_blend_splits: boolean;
  sky_mode: number;
}

export declare class ENetPacketPeer extends PacketPeer {
  peer_disconnect(data?: number): void;
  peer_disconnect_later(data?: number): void;
  peer_disconnect_now(data?: number): void;
  ping(): void;
  ping_interval(ping_interval: number): void;
  reset(): void;
  send(channel: number, packet: PackedByteArray, flags: number): number;
  throttle_configure(interval: number, acceleration: number, deceleration: number): void;
  set_timeout(timeout: number, timeout_min: number, timeout_max: number): void;
  get_packet_flags(): number;
  get_remote_address(): string;
  get_remote_port(): number;
  get_statistic(statistic: number): number;
  get_state(): number;
  get_channels(): number;
  is_active(): boolean;
}

export declare class EncodedObjectAsID extends RefCounted {
  object_id: number;
}

export declare class Environment extends Resource {
  background_mode: number;
  background_color: Color;
  background_energy_multiplier: number;
  background_intensity: number;
  background_canvas_max_layer: number;
  background_camera_feed_id: number;
  sky: Sky;
  sky_custom_fov: number;
  sky_rotation: Vector3;
  ambient_light_source: number;
  ambient_light_color: Color;
  ambient_light_sky_contribution: number;
  ambient_light_energy: number;
  reflected_light_source: number;
  tonemap_mode: number;
  tonemap_exposure: number;
  tonemap_white: number;
  ssr_enabled: boolean;
  ssr_max_steps: number;
  ssr_fade_in: number;
  ssr_fade_out: number;
  ssr_depth_tolerance: number;
  ssao_enabled: boolean;
  ssao_radius: number;
  ssao_intensity: number;
  ssao_power: number;
  ssao_detail: number;
  ssao_horizon: number;
  ssao_sharpness: number;
  ssao_light_affect: number;
  ssao_ao_channel_affect: number;
  ssil_enabled: boolean;
  ssil_radius: number;
  ssil_intensity: number;
  ssil_sharpness: number;
  ssil_normal_rejection: number;
  sdfgi_enabled: boolean;
  sdfgi_use_occlusion: boolean;
  sdfgi_read_sky_light: boolean;
  sdfgi_bounce_feedback: number;
  sdfgi_cascades: number;
  sdfgi_min_cell_size: number;
  sdfgi_cascade0_distance: number;
  sdfgi_max_distance: number;
  sdfgi_y_scale: number;
  sdfgi_energy: number;
  sdfgi_normal_bias: number;
  sdfgi_probe_bias: number;
  glow_enabled: boolean;
  glow_normalized: boolean;
  glow_intensity: number;
  glow_strength: number;
  glow_mix: number;
  glow_bloom: number;
  glow_blend_mode: number;
  glow_hdr_threshold: number;
  glow_hdr_scale: number;
  glow_hdr_luminance_cap: number;
  glow_map_strength: number;
  fog_enabled: boolean;
  fog_mode: number;
  fog_light_color: Color;
  fog_light_energy: number;
  fog_sun_scatter: number;
  fog_density: number;
  fog_aerial_perspective: number;
  fog_sky_affect: number;
  fog_height: number;
  fog_height_density: number;
  fog_depth_curve: number;
  fog_depth_begin: number;
  fog_depth_end: number;
  volumetric_fog_enabled: boolean;
  volumetric_fog_density: number;
  volumetric_fog_albedo: Color;
  volumetric_fog_emission: Color;
  volumetric_fog_emission_energy: number;
  volumetric_fog_gi_inject: number;
  volumetric_fog_anisotropy: number;
  volumetric_fog_length: number;
  volumetric_fog_detail_spread: number;
  volumetric_fog_ambient_inject: number;
  volumetric_fog_sky_affect: number;
  volumetric_fog_temporal_reprojection_enabled: boolean;
  volumetric_fog_temporal_reprojection_amount: number;
  adjustment_enabled: boolean;
  adjustment_brightness: number;
  adjustment_contrast: number;
  adjustment_saturation: number;
  set_glow_level(idx: number, intensity: number): void;
  get_glow_level(idx: number): number;
}

export declare class ExternalTexture extends Texture2D {
  size: Vector2;
  get_external_texture_id(): number;
  set_external_buffer_id(external_buffer_id: number): void;
}

export declare class FBXDocument extends GLTFDocument {
}

export declare class FBXState extends GLTFState {
  allow_geometry_helper_nodes: boolean;
}

export declare class FastNoiseLite extends Noise {
  noise_type: number;
  seed: number;
  frequency: number;
  offset: Vector3;
  fractal_type: number;
  fractal_octaves: number;
  fractal_lacunarity: number;
  fractal_gain: number;
  fractal_weighted_strength: number;
  fractal_ping_pong_strength: number;
  cellular_distance_function: number;
  cellular_jitter: number;
  cellular_return_type: number;
  domain_warp_enabled: boolean;
  domain_warp_type: number;
  domain_warp_amplitude: number;
  domain_warp_frequency: number;
  domain_warp_fractal_type: number;
  domain_warp_fractal_octaves: number;
  domain_warp_fractal_lacunarity: number;
  domain_warp_fractal_gain: number;
}

export declare class FileDialog extends ConfirmationDialog {
  mode_overrides_title: boolean;
  file_mode: number;
  display_mode: number;
  access: number;
  root_subfolder: string;
  filters: PackedStringArray;
  filename_filter: string;
  show_hidden_files: boolean;
  use_native_dialog: boolean;
  option_count: number;
  current_dir: string;
  current_file: string;
  current_path: string;
  clear_filters(): void;
  add_filter(filter: string, description?: string): void;
  clear_filename_filter(): void;
  get_option_name(option: number): string;
  get_option_values(option: number): PackedStringArray;
  get_option_default(option: number): number;
  set_option_name(option: number, name: string): void;
  set_option_values(option: number, values: PackedStringArray): void;
  set_option_default(option: number, default_value_index: number): void;
  add_option(name: string, values: PackedStringArray, default_value_index: number): void;
  get_selected_options(): Record<string, any>;
  get_vbox(): VBoxContainer;
  get_line_edit(): LineEdit;
  deselect_all(): void;
  invalidate(): void;
}

export declare class FileSystemDock extends VBoxContainer {
  navigate_to_path(path: string): void;
  add_resource_tooltip_plugin(plugin: EditorResourceTooltipPlugin): void;
  remove_resource_tooltip_plugin(plugin: EditorResourceTooltipPlugin): void;
}

export declare class FlowContainer extends Container {
  alignment: number;
  last_wrap_alignment: number;
  vertical: boolean;
  reverse_fill: boolean;
  get_line_count(): number;
}

export declare class FogMaterial extends Material {
  density: number;
  albedo: Color;
  emission: Color;
  height_falloff: number;
  edge_fade: number;
  density_texture: Texture3D;
}

export declare class FogVolume extends VisualInstance3D {
  size: Vector3;
  shape: number;
}

export declare class FoldableContainer extends Container {
  folded: boolean;
  title: string;
  title_alignment: number;
  title_position: number;
  title_text_overrun_behavior: number;
  foldable_group: FoldableGroup;
  title_text_direction: number;
  language: string;
  fold(): void;
  expand(): void;
  add_title_bar_control(control: Control): void;
  remove_title_bar_control(control: Control): void;
}

export declare class FoldableGroup extends Resource {
  allow_folding_all: boolean;
  get_expanded_container(): FoldableContainer;
  get_containers(): any[];
}

export declare class Font extends Resource {
  find_variation(variation_coordinates: Record<string, any>, face_index?: number, strength?: number, transform?: Transform2D, spacing_top?: number, spacing_bottom?: number, spacing_space?: number, spacing_glyph?: number, baseline_offset?: number): RID;
  get_rids(): any[];
  get_height(font_size?: number): number;
  get_ascent(font_size?: number): number;
  get_descent(font_size?: number): number;
  get_underline_position(font_size?: number): number;
  get_underline_thickness(font_size?: number): number;
  get_font_name(): string;
  get_font_style_name(): string;
  get_ot_name_strings(): Record<string, any>;
  get_font_style(): number;
  get_font_weight(): number;
  get_font_stretch(): number;
  get_spacing(spacing: number): number;
  get_opentype_features(): Record<string, any>;
  set_cache_capacity(single_line: number, multi_line: number): void;
  get_string_size(text: string, alignment?: number, width?: number, font_size?: number, justification_flags?: number, direction?: number, orientation?: number): Vector2;
  get_multiline_string_size(text: string, alignment?: number, width?: number, font_size?: number, max_lines?: number, brk_flags?: number, justification_flags?: number, direction?: number, orientation?: number): Vector2;
  draw_string(canvas_item: RID, pos: Vector2, text: string, alignment?: number, width?: number, font_size?: number, modulate?: Color, justification_flags?: number, direction?: number, orientation?: number, oversampling?: number): void;
  draw_multiline_string(canvas_item: RID, pos: Vector2, text: string, alignment?: number, width?: number, font_size?: number, max_lines?: number, modulate?: Color, brk_flags?: number, justification_flags?: number, direction?: number, orientation?: number, oversampling?: number): void;
  draw_string_outline(canvas_item: RID, pos: Vector2, text: string, alignment?: number, width?: number, font_size?: number, size?: number, modulate?: Color, justification_flags?: number, direction?: number, orientation?: number, oversampling?: number): void;
  draw_multiline_string_outline(canvas_item: RID, pos: Vector2, text: string, alignment?: number, width?: number, font_size?: number, max_lines?: number, size?: number, modulate?: Color, brk_flags?: number, justification_flags?: number, direction?: number, orientation?: number, oversampling?: number): void;
  get_char_size(char: number, font_size: number): Vector2;
  draw_char(canvas_item: RID, pos: Vector2, char: number, font_size: number, modulate?: Color, oversampling?: number): number;
  draw_char_outline(canvas_item: RID, pos: Vector2, char: number, font_size: number, size?: number, modulate?: Color, oversampling?: number): number;
  has_char(char: number): boolean;
  get_supported_chars(): string;
  is_language_supported(language: string): boolean;
  is_script_supported(script: string): boolean;
  get_supported_feature_list(): Record<string, any>;
  get_supported_variation_list(): Record<string, any>;
  get_face_count(): number;
}

export declare class FontFile extends Font {
  data: PackedByteArray;
  generate_mipmaps: boolean;
  disable_embedded_bitmaps: boolean;
  antialiasing: number;
  font_name: string;
  style_name: string;
  font_style: number;
  font_weight: number;
  font_stretch: number;
  subpixel_positioning: number;
  keep_rounding_remainders: boolean;
  multichannel_signed_distance_field: boolean;
  msdf_pixel_range: number;
  msdf_size: number;
  allow_system_fallback: boolean;
  force_autohinter: boolean;
  modulate_color_glyphs: boolean;
  hinting: number;
  fixed_size: number;
  fixed_size_scale_mode: number;
  opentype_feature_overrides: Record<string, any>;
  oversampling: number;
  load_bitmap_font(path: string): number;
  load_dynamic_font(path: string): number;
  get_cache_count(): number;
  clear_cache(): void;
  remove_cache(cache_index: number): void;
  get_size_cache_list(cache_index: number): any[];
  clear_size_cache(cache_index: number): void;
  remove_size_cache(cache_index: number, size: Vector2i): void;
  set_variation_coordinates(cache_index: number, variation_coordinates: Record<string, any>): void;
  get_variation_coordinates(cache_index: number): Record<string, any>;
  set_embolden(cache_index: number, strength: number): void;
  get_embolden(cache_index: number): number;
  set_transform(cache_index: number, transform: Transform2D): void;
  get_transform(cache_index: number): Transform2D;
  set_extra_spacing(cache_index: number, spacing: number, value: number): void;
  get_extra_spacing(cache_index: number, spacing: number): number;
  set_extra_baseline_offset(cache_index: number, baseline_offset: number): void;
  get_extra_baseline_offset(cache_index: number): number;
  set_face_index(cache_index: number, face_index: number): void;
  get_face_index(cache_index: number): number;
  set_cache_ascent(cache_index: number, size: number, ascent: number): void;
  get_cache_ascent(cache_index: number, size: number): number;
  set_cache_descent(cache_index: number, size: number, descent: number): void;
  get_cache_descent(cache_index: number, size: number): number;
  set_cache_underline_position(cache_index: number, size: number, underline_position: number): void;
  get_cache_underline_position(cache_index: number, size: number): number;
  set_cache_underline_thickness(cache_index: number, size: number, underline_thickness: number): void;
  get_cache_underline_thickness(cache_index: number, size: number): number;
  set_cache_scale(cache_index: number, size: number, scale: number): void;
  get_cache_scale(cache_index: number, size: number): number;
  get_texture_count(cache_index: number, size: Vector2i): number;
  clear_textures(cache_index: number, size: Vector2i): void;
  remove_texture(cache_index: number, size: Vector2i, texture_index: number): void;
  set_texture_image(cache_index: number, size: Vector2i, texture_index: number, image: Image): void;
  get_texture_image(cache_index: number, size: Vector2i, texture_index: number): Image;
  set_texture_offsets(cache_index: number, size: Vector2i, texture_index: number, offset: PackedInt32Array): void;
  get_texture_offsets(cache_index: number, size: Vector2i, texture_index: number): PackedInt32Array;
  get_glyph_list(cache_index: number, size: Vector2i): PackedInt32Array;
  clear_glyphs(cache_index: number, size: Vector2i): void;
  remove_glyph(cache_index: number, size: Vector2i, glyph: number): void;
  set_glyph_advance(cache_index: number, size: number, glyph: number, advance: Vector2): void;
  get_glyph_advance(cache_index: number, size: number, glyph: number): Vector2;
  set_glyph_offset(cache_index: number, size: Vector2i, glyph: number, offset: Vector2): void;
  get_glyph_offset(cache_index: number, size: Vector2i, glyph: number): Vector2;
  set_glyph_size(cache_index: number, size: Vector2i, glyph: number, gl_size: Vector2): void;
  get_glyph_size(cache_index: number, size: Vector2i, glyph: number): Vector2;
  set_glyph_uv_rect(cache_index: number, size: Vector2i, glyph: number, uv_rect: Rect2): void;
  get_glyph_uv_rect(cache_index: number, size: Vector2i, glyph: number): Rect2;
  set_glyph_texture_idx(cache_index: number, size: Vector2i, glyph: number, texture_idx: number): void;
  get_glyph_texture_idx(cache_index: number, size: Vector2i, glyph: number): number;
  get_kerning_list(cache_index: number, size: number): any[];
  clear_kerning_map(cache_index: number, size: number): void;
  remove_kerning(cache_index: number, size: number, glyph_pair: Vector2i): void;
  set_kerning(cache_index: number, size: number, glyph_pair: Vector2i, kerning: Vector2): void;
  get_kerning(cache_index: number, size: number, glyph_pair: Vector2i): Vector2;
  render_range(cache_index: number, size: Vector2i, start: number, end: number): void;
  render_glyph(cache_index: number, size: Vector2i, index: number): void;
  set_language_support_override(language: string, supported: boolean): void;
  get_language_support_override(language: string): boolean;
  remove_language_support_override(language: string): void;
  get_language_support_overrides(): PackedStringArray;
  set_script_support_override(script: string, supported: boolean): void;
  get_script_support_override(script: string): boolean;
  remove_script_support_override(script: string): void;
  get_script_support_overrides(): PackedStringArray;
  get_glyph_index(size: number, char: number, variation_selector: number): number;
  get_char_from_glyph_index(size: number, glyph_index: number): number;
}

export declare class FontVariation extends Font {
  base_font: Font;
  variation_opentype: Record<string, any>;
  variation_face_index: number;
  variation_embolden: number;
  variation_transform: Transform2D;
  opentype_features: Record<string, any>;
  baseline_offset: number;
}

export declare class FramebufferCacheRD extends Object {
  get_cache_multipass(textures: any[], passes: any[], views: number): RID;
}

export declare class GDScriptSyntaxHighlighter extends EditorSyntaxHighlighter {
}

export declare class GLTFAccessor extends Resource {
  buffer_view: number;
  byte_offset: number;
  component_type: number;
  normalized: boolean;
  count: number;
  accessor_type: number;
  type: number;
  min: PackedFloat64Array;
  max: PackedFloat64Array;
  sparse_count: number;
  sparse_indices_buffer_view: number;
  sparse_indices_byte_offset: number;
  sparse_indices_component_type: number;
  sparse_values_buffer_view: number;
  sparse_values_byte_offset: number;
}

export declare class GLTFAnimation extends Resource {
  original_name: string;
  loop: boolean;
  get_additional_data(extension_name: string): any;
  set_additional_data(extension_name: string, additional_data: any): void;
}

export declare class GLTFBufferView extends Resource {
  buffer: number;
  byte_offset: number;
  byte_length: number;
  byte_stride: number;
  indices: boolean;
  vertex_attributes: boolean;
  load_buffer_view_data(state: GLTFState): PackedByteArray;
}

export declare class GLTFCamera extends Resource {
  perspective: boolean;
  fov: number;
  size_mag: number;
  depth_far: number;
  depth_near: number;
  from_node(camera_node: Camera3D): GLTFCamera;
  to_node(): Camera3D;
  from_dictionary(dictionary: Record<string, any>): GLTFCamera;
  to_dictionary(): Record<string, any>;
}

export declare class GLTFDocument extends Resource {
  image_format: string;
  lossy_quality: number;
  fallback_image_format: string;
  fallback_image_quality: number;
  root_node_mode: number;
  visibility_mode: number;
  append_from_file(path: string, state: GLTFState, flags?: number, base_path?: string): number;
  append_from_buffer(bytes: PackedByteArray, base_path: string, state: GLTFState, flags?: number): number;
  append_from_scene(node: Node, state: GLTFState, flags?: number): number;
  generate_scene(state: GLTFState, bake_fps?: number, trimming?: boolean, remove_immutable_tracks?: boolean): Node;
  generate_buffer(state: GLTFState): PackedByteArray;
  write_to_filesystem(state: GLTFState, path: string): number;
  import_object_model_property(state: GLTFState, json_pointer: string): GLTFObjectModelProperty;
  export_object_model_property(state: GLTFState, node_path: string, godot_node: Node, gltf_node_index: number): GLTFObjectModelProperty;
  register_gltf_document_extension(extension: GLTFDocumentExtension, first_priority?: boolean): void;
  unregister_gltf_document_extension(extension: GLTFDocumentExtension): void;
  get_supported_gltf_extensions(): PackedStringArray;
}

export declare class GLTFDocumentExtension extends Resource {
}

export declare class GLTFDocumentExtensionConvertImporterMesh extends GLTFDocumentExtension {
}

export declare class GLTFLight extends Resource {
  color: Color;
  intensity: number;
  light_type: string;
  range: number;
  inner_cone_angle: number;
  outer_cone_angle: number;
  from_node(light_node: Light3D): GLTFLight;
  to_node(): Light3D;
  from_dictionary(dictionary: Record<string, any>): GLTFLight;
  to_dictionary(): Record<string, any>;
  get_additional_data(extension_name: string): any;
  set_additional_data(extension_name: string, additional_data: any): void;
}

export declare class GLTFMesh extends Resource {
  original_name: string;
  blend_weights: PackedFloat32Array;
  get_additional_data(extension_name: string): any;
  set_additional_data(extension_name: string, additional_data: any): void;
}

export declare class GLTFNode extends Resource {
  original_name: string;
  parent: number;
  height: number;
  xform: Transform3D;
  mesh: number;
  camera: number;
  skin: number;
  skeleton: number;
  position: Vector3;
  rotation: Quaternion;
  scale: Vector3;
  children: PackedInt32Array;
  light: number;
  visible: boolean;
  append_child_index(child_index: number): void;
  get_additional_data(extension_name: string): any;
  set_additional_data(extension_name: string, additional_data: any): void;
  get_scene_node_path(gltf_state: GLTFState, handle_skeletons?: boolean): string;
}

export declare class GLTFObjectModelProperty extends RefCounted {
  gltf_to_godot_expression: Expression;
  godot_to_gltf_expression: Expression;
  object_model_type: number;
  variant_type: number;
  append_node_path(node_path: string): void;
  append_path_to_property(node_path: string, prop_name: string): void;
  get_accessor_type(): number;
  has_node_paths(): boolean;
  has_json_pointers(): boolean;
  set_types(variant_type: number, obj_model_type: number): void;
}

export declare class GLTFPhysicsBody extends Resource {
  body_type: string;
  mass: number;
  linear_velocity: Vector3;
  angular_velocity: Vector3;
  center_of_mass: Vector3;
  inertia_diagonal: Vector3;
  inertia_orientation: Quaternion;
  inertia_tensor: Basis;
  from_node(body_node: CollisionObject3D): GLTFPhysicsBody;
  to_node(): CollisionObject3D;
  from_dictionary(dictionary: Record<string, any>): GLTFPhysicsBody;
  to_dictionary(): Record<string, any>;
}

export declare class GLTFPhysicsShape extends Resource {
  shape_type: string;
  size: Vector3;
  radius: number;
  height: number;
  is_trigger: boolean;
  mesh_index: number;
  importer_mesh: ImporterMesh;
  from_node(shape_node: CollisionShape3D): GLTFPhysicsShape;
  to_node(cache_shapes?: boolean): CollisionShape3D;
  from_resource(shape_resource: Shape3D): GLTFPhysicsShape;
  to_resource(cache_shapes?: boolean): Shape3D;
  from_dictionary(dictionary: Record<string, any>): GLTFPhysicsShape;
  to_dictionary(): Record<string, any>;
}

export declare class GLTFSkeleton extends Resource {
  joints: PackedInt32Array;
  roots: PackedInt32Array;
  godot_bone_node: Record<string, any>;
  get_godot_skeleton(): Skeleton3D;
  get_bone_attachment_count(): number;
  get_bone_attachment(idx: number): BoneAttachment3D;
}

export declare class GLTFSkin extends Resource {
  skin_root: number;
  joints_original: PackedInt32Array;
  joints: PackedInt32Array;
  non_joints: PackedInt32Array;
  roots: PackedInt32Array;
  skeleton: number;
  joint_i_to_bone_i: Record<string, any>;
  joint_i_to_name: Record<string, any>;
  godot_skin: Skin;
}

export declare class GLTFSpecGloss extends Resource {
  diffuse_factor: Color;
  gloss_factor: number;
  specular_factor: Color;
}

export declare class GLTFState extends Resource {
  json: Record<string, any>;
  major_version: number;
  minor_version: number;
  copyright: string;
  glb_data: PackedByteArray;
  use_named_skin_binds: boolean;
  scene_name: string;
  base_path: string;
  filename: string;
  root_nodes: PackedInt32Array;
  create_animations: boolean;
  import_as_skeleton_bones: boolean;
  handle_binary_image: number;
  bake_fps: number;
  add_used_extension(extension_name: string, required: boolean): void;
  append_data_to_buffers(data: PackedByteArray, deduplication: boolean): number;
  append_gltf_node(gltf_node: GLTFNode, godot_scene_node: Node, parent_node_index: number): number;
  get_animation_players_count(idx: number): number;
  get_animation_player(idx: number): AnimationPlayer;
  get_scene_node(idx: number): Node;
  get_node_index(scene_node: Node): number;
  get_additional_data(extension_name: string): any;
  set_additional_data(extension_name: string, additional_data: any): void;
}

export declare class GLTFTexture extends Resource {
  src_image: number;
  sampler: number;
}

export declare class GLTFTextureSampler extends Resource {
  mag_filter: number;
  min_filter: number;
  wrap_s: number;
  wrap_t: number;
}

export declare class GPUParticles2D extends Node2D {
  emitting: boolean;
  amount: number;
  amount_ratio: number;
  sub_emitter: string;
  texture: Texture2D;
  lifetime: number;
  interp_to_end: number;
  one_shot: boolean;
  preprocess: number;
  speed_scale: number;
  explosiveness: number;
  randomness: number;
  use_fixed_seed: boolean;
  seed: number;
  fixed_fps: number;
  interpolate: boolean;
  fract_delta: boolean;
  collision_base_size: number;
  visibility_rect: Rect2;
  local_coords: boolean;
  draw_order: number;
  trail_enabled: boolean;
  trail_lifetime: number;
  trail_sections: number;
  trail_section_subdivisions: number;
  request_particles_process(process_time: number): void;
  capture_rect(): Rect2;
  restart(keep_seed?: boolean): void;
  emit_particle(xform: Transform2D, velocity: Vector2, color: Color, custom: Color, flags: number): void;
  convert_from_particles(particles: Node): void;
}

export declare class GPUParticles3D extends GeometryInstance3D {
  emitting: boolean;
  amount: number;
  amount_ratio: number;
  sub_emitter: string;
  lifetime: number;
  interp_to_end: number;
  one_shot: boolean;
  preprocess: number;
  speed_scale: number;
  explosiveness: number;
  randomness: number;
  use_fixed_seed: boolean;
  seed: number;
  fixed_fps: number;
  interpolate: boolean;
  fract_delta: boolean;
  collision_base_size: number;
  visibility_aabb: AABB;
  local_coords: boolean;
  draw_order: number;
  transform_align: number;
  trail_enabled: boolean;
  trail_lifetime: number;
  draw_passes: number;
  draw_skin: Skin;
  restart(keep_seed?: boolean): void;
  capture_aabb(): AABB;
  emit_particle(xform: Transform3D, velocity: Vector3, color: Color, custom: Color, flags: number): void;
  convert_from_particles(particles: Node): void;
  request_particles_process(process_time: number): void;
}

export declare class GPUParticlesAttractor3D extends VisualInstance3D {
  strength: number;
  attenuation: number;
  directionality: number;
  cull_mask: number;
}

export declare class GPUParticlesAttractorBox3D extends GPUParticlesAttractor3D {
  size: Vector3;
}

export declare class GPUParticlesAttractorSphere3D extends GPUParticlesAttractor3D {
  radius: number;
}

export declare class GPUParticlesAttractorVectorField3D extends GPUParticlesAttractor3D {
  size: Vector3;
  texture: Texture3D;
}

export declare class GPUParticlesCollision3D extends VisualInstance3D {
  cull_mask: number;
}

export declare class GPUParticlesCollisionBox3D extends GPUParticlesCollision3D {
  size: Vector3;
}

export declare class GPUParticlesCollisionHeightField3D extends GPUParticlesCollision3D {
  size: Vector3;
  resolution: number;
  update_mode: number;
  follow_camera_enabled: boolean;
  heightfield_mask: number;
  set_heightfield_mask_value(layer_number: number, value: boolean): void;
  get_heightfield_mask_value(layer_number: number): boolean;
}

export declare class GPUParticlesCollisionSDF3D extends GPUParticlesCollision3D {
  size: Vector3;
  resolution: number;
  thickness: number;
  bake_mask: number;
  texture: Texture3D;
  set_bake_mask_value(layer_number: number, value: boolean): void;
  get_bake_mask_value(layer_number: number): boolean;
}

export declare class GPUParticlesCollisionSphere3D extends GPUParticlesCollision3D {
  radius: number;
}

export declare class Generic6DOFJoint3D extends Joint3D {
  set_param_x(param: number, value: number): void;
  get_param_x(param: number): number;
  set_param_y(param: number, value: number): void;
  get_param_y(param: number): number;
  set_param_z(param: number, value: number): void;
  get_param_z(param: number): number;
  set_flag_x(flag: number, value: boolean): void;
  get_flag_x(flag: number): boolean;
  set_flag_y(flag: number, value: boolean): void;
  get_flag_y(flag: number): boolean;
  set_flag_z(flag: number, value: boolean): void;
  get_flag_z(flag: number): boolean;
}

export declare class GeometryInstance3D extends VisualInstance3D {
  transparency: number;
  cast_shadow: number;
  extra_cull_margin: number;
  custom_aabb: AABB;
  lod_bias: number;
  ignore_occlusion_culling: boolean;
  gi_mode: number;
  gi_lightmap_texel_scale: number;
  gi_lightmap_scale: number;
  visibility_range_begin: number;
  visibility_range_begin_margin: number;
  visibility_range_end: number;
  visibility_range_end_margin: number;
  visibility_range_fade_mode: number;
  set_instance_shader_parameter(name: string, value: any): void;
  get_instance_shader_parameter(name: string): any;
}

export declare class Gradient extends Resource {
  interpolation_mode: number;
  interpolation_color_space: number;
  offsets: PackedFloat32Array;
  colors: PackedColorArray;
  add_point(offset: number, color: Color): void;
  remove_point(point: number): void;
  set_offset(point: number, offset: number): void;
  get_offset(point: number): number;
  reverse(): void;
  set_color(point: number, color: Color): void;
  get_color(point: number): Color;
  sample(offset: number): Color;
  get_point_count(): number;
}

export declare class GradientTexture1D extends Texture2D {
  gradient: Gradient;
  width: number;
  use_hdr: boolean;
}

export declare class GradientTexture2D extends Texture2D {
  gradient: Gradient;
  width: number;
  height: number;
  use_hdr: boolean;
  fill: number;
  fill_from: Vector2;
  fill_to: Vector2;
  repeat: number;
}

export declare class GraphEdit extends Control {
  scroll_offset: Vector2;
  show_grid: boolean;
  grid_pattern: number;
  snapping_enabled: boolean;
  snapping_distance: number;
  panning_scheme: number;
  right_disconnects: boolean;
  connection_lines_curvature: number;
  connection_lines_thickness: number;
  connection_lines_antialiased: boolean;
  zoom: number;
  zoom_min: number;
  zoom_max: number;
  zoom_step: number;
  minimap_enabled: boolean;
  minimap_size: Vector2;
  minimap_opacity: number;
  show_menu: boolean;
  show_zoom_label: boolean;
  show_zoom_buttons: boolean;
  show_grid_buttons: boolean;
  show_minimap_button: boolean;
  show_arrange_button: boolean;
  connect_node(from_node: string, from_port: number, to_node: string, to_port: number, keep_alive?: boolean): number;
  is_node_connected(from_node: string, from_port: number, to_node: string, to_port: number): boolean;
  disconnect_node(from_node: string, from_port: number, to_node: string, to_port: number): void;
  set_connection_activity(from_node: string, from_port: number, to_node: string, to_port: number, amount: number): void;
  get_connection_count(from_node: string, from_port: number): number;
  get_closest_connection_at_point(point: Vector2, max_distance?: number): Record<string, any>;
  get_connection_list_from_node(node: string): any[];
  get_connections_intersecting_with_rect(rect: Rect2): any[];
  clear_connections(): void;
  force_connection_drag_end(): void;
  add_valid_right_disconnect_type(type: number): void;
  remove_valid_right_disconnect_type(type: number): void;
  add_valid_left_disconnect_type(type: number): void;
  remove_valid_left_disconnect_type(type: number): void;
  add_valid_connection_type(from_type: number, to_type: number): void;
  remove_valid_connection_type(from_type: number, to_type: number): void;
  is_valid_connection_type(from_type: number, to_type: number): boolean;
  get_connection_line(from_node: Vector2, to_node: Vector2): PackedVector2Array;
  attach_graph_element_to_frame(element: string, frame: string): void;
  detach_graph_element_from_frame(element: string): void;
  get_element_frame(element: string): GraphFrame;
  get_attached_nodes_of_frame(frame: string): any[];
  get_menu_hbox(): HBoxContainer;
  arrange_nodes(): void;
  set_selected(node: Node): void;
}

export declare class GraphElement extends Container {
  position_offset: Vector2;
  resizable: boolean;
  draggable: boolean;
  selectable: boolean;
  selected: boolean;
}

export declare class GraphFrame extends GraphElement {
  title: string;
  autoshrink_enabled: boolean;
  autoshrink_margin: number;
  drag_margin: number;
  tint_color_enabled: boolean;
  tint_color: Color;
  get_titlebar_hbox(): HBoxContainer;
}

export declare class GraphNode extends GraphElement {
  title: string;
  ignore_invalid_connection_type: boolean;
  slots_focus_mode: number;
  get_titlebar_hbox(): HBoxContainer;
  set_slot(slot_index: number, enable_left_port: boolean, type_left: number, color_left: Color, enable_right_port: boolean, type_right: number, color_right: Color, custom_icon_left?: Texture2D, custom_icon_right?: Texture2D, draw_stylebox?: boolean): void;
  clear_slot(slot_index: number): void;
  clear_all_slots(): void;
  is_slot_enabled_left(slot_index: number): boolean;
  set_slot_enabled_left(slot_index: number, enable: boolean): void;
  set_slot_type_left(slot_index: number, type: number): void;
  get_slot_type_left(slot_index: number): number;
  set_slot_color_left(slot_index: number, color: Color): void;
  get_slot_color_left(slot_index: number): Color;
  set_slot_custom_icon_left(slot_index: number, custom_icon: Texture2D): void;
  get_slot_custom_icon_left(slot_index: number): Texture2D;
  is_slot_enabled_right(slot_index: number): boolean;
  set_slot_enabled_right(slot_index: number, enable: boolean): void;
  set_slot_type_right(slot_index: number, type: number): void;
  get_slot_type_right(slot_index: number): number;
  set_slot_color_right(slot_index: number, color: Color): void;
  get_slot_color_right(slot_index: number): Color;
  set_slot_custom_icon_right(slot_index: number, custom_icon: Texture2D): void;
  get_slot_custom_icon_right(slot_index: number): Texture2D;
  is_slot_draw_stylebox(slot_index: number): boolean;
  set_slot_draw_stylebox(slot_index: number, enable: boolean): void;
  get_input_port_count(): number;
  get_input_port_position(port_idx: number): Vector2;
  get_input_port_type(port_idx: number): number;
  get_input_port_color(port_idx: number): Color;
  get_input_port_slot(port_idx: number): number;
  get_output_port_count(): number;
  get_output_port_position(port_idx: number): Vector2;
  get_output_port_type(port_idx: number): number;
  get_output_port_color(port_idx: number): Color;
  get_output_port_slot(port_idx: number): number;
}

export declare class GridContainer extends Container {
  columns: number;
}

export declare class GridMap extends Node3D {
  mesh_library: MeshLibrary;
  physics_material: PhysicsMaterial;
  cell_size: Vector3;
  cell_octant_size: number;
  cell_center_x: boolean;
  cell_center_y: boolean;
  cell_center_z: boolean;
  cell_scale: number;
  collision_layer: number;
  collision_mask: number;
  collision_priority: number;
  bake_navigation: boolean;
  set_collision_mask_value(layer_number: number, value: boolean): void;
  get_collision_mask_value(layer_number: number): boolean;
  set_collision_layer_value(layer_number: number, value: boolean): void;
  get_collision_layer_value(layer_number: number): boolean;
  set_navigation_map(navigation_map: RID): void;
  get_navigation_map(): RID;
  set_cell_item(position: Vector3i, item: number, orientation?: number): void;
  get_cell_item(position: Vector3i): number;
  get_cell_item_orientation(position: Vector3i): number;
  get_cell_item_basis(position: Vector3i): Basis;
  get_basis_with_orthogonal_index(index: number): Basis;
  get_orthogonal_index_from_basis(basis: Basis): number;
  local_to_map(local_position: Vector3): Vector3i;
  map_to_local(map_position: Vector3i): Vector3;
  resource_changed(resource: Resource): void;
  clear(): void;
  get_used_cells(): any[];
  get_used_cells_by_item(item: number): any[];
  get_meshes(): any[];
  get_bake_meshes(): any[];
  get_bake_mesh_instance(idx: number): RID;
  clear_baked_meshes(): void;
  make_baked_meshes(gen_lightmap_uv?: boolean, lightmap_uv_texel_size?: number): void;
}

export declare class GridMapEditorPlugin extends EditorPlugin {
  get_current_grid_map(): GridMap;
  set_selection(begin: Vector3i, end: Vector3i): void;
  clear_selection(): void;
  get_selection(): AABB;
  has_selection(): boolean;
  get_selected_cells(): any[];
  set_selected_palette_item(item: number): void;
  get_selected_palette_item(): number;
}

export declare class GrooveJoint2D extends Joint2D {
  length: number;
  initial_offset: number;
}

export declare class HBoxContainer extends BoxContainer {
}

export declare class HFlowContainer extends FlowContainer {
}

export declare class HMACContext extends RefCounted {
  start(hash_type: number, key: PackedByteArray): number;
  update(data: PackedByteArray): number;
  finish(): PackedByteArray;
}

export declare class HScrollBar extends ScrollBar {
}

export declare class HSeparator extends Separator {
}

export declare class HSlider extends Slider {
}

export declare class HSplitContainer extends SplitContainer {
}

export declare class HashingContext extends RefCounted {
  start(type: number): number;
  update(chunk: PackedByteArray): number;
  finish(): PackedByteArray;
}

export declare class HeightMapShape3D extends Shape3D {
  map_width: number;
  map_depth: number;
  map_data: PackedFloat32Array;
  get_min_height(): number;
  get_max_height(): number;
  update_map_data_from_image(image: Image, height_min: number, height_max: number): void;
}

export declare class HingeJoint3D extends Joint3D {
  set_param(param: number, value: number): void;
  get_param(param: number): number;
  set_flag(flag: number, enabled: boolean): void;
  get_flag(flag: number): boolean;
}

export declare class Image extends Resource {
  get_width(): number;
  get_height(): number;
  get_size(): Vector2i;
  has_mipmaps(): boolean;
  get_format(): number;
  get_data_size(): number;
  convert(format: number): void;
  get_mipmap_count(): number;
  get_mipmap_offset(mipmap: number): number;
  resize_to_po2(square?: boolean, interpolation?: number): void;
  resize(width: number, height: number, interpolation?: number): void;
  shrink_x2(): void;
  crop(width: number, height: number): void;
  flip_x(): void;
  flip_y(): void;
  generate_mipmaps(renormalize?: boolean): number;
  clear_mipmaps(): void;
  create(width: number, height: number, use_mipmaps: boolean, format: number): Image;
  create_empty(width: number, height: number, use_mipmaps: boolean, format: number): Image;
  create_from_data(width: number, height: number, use_mipmaps: boolean, format: number, data: PackedByteArray): Image;
  is_empty(): boolean;
  load(path: string): number;
  load_from_file(path: string): Image;
  save_png(path: string): number;
  save_png_to_buffer(): PackedByteArray;
  save_jpg(path: string, quality?: number): number;
  save_jpg_to_buffer(quality?: number): PackedByteArray;
  save_exr(path: string, grayscale?: boolean): number;
  save_exr_to_buffer(grayscale?: boolean): PackedByteArray;
  save_dds(path: string): number;
  save_dds_to_buffer(): PackedByteArray;
  save_webp(path: string, lossy?: boolean, quality?: number): number;
  save_webp_to_buffer(lossy?: boolean, quality?: number): PackedByteArray;
  detect_alpha(): number;
  is_invisible(): boolean;
  detect_used_channels(source?: number): number;
  compress(mode: number, source?: number, astc_format?: number): number;
  compress_from_channels(mode: number, channels: number, astc_format?: number): number;
  decompress(): number;
  is_compressed(): boolean;
  rotate_90(direction: number): void;
  rotate_180(): void;
  fix_alpha_edges(): void;
  premultiply_alpha(): void;
  srgb_to_linear(): void;
  linear_to_srgb(): void;
  normal_map_to_xy(): void;
  rgbe_to_srgb(): Image;
  bump_map_to_normal_map(bump_scale?: number): void;
  compute_image_metrics(compared_image: Image, use_luma: boolean): Record<string, any>;
  blit_rect(src: Image, src_rect: Rect2i, dst: Vector2i): void;
  blit_rect_mask(src: Image, mask: Image, src_rect: Rect2i, dst: Vector2i): void;
  blend_rect(src: Image, src_rect: Rect2i, dst: Vector2i): void;
  blend_rect_mask(src: Image, mask: Image, src_rect: Rect2i, dst: Vector2i): void;
  fill(color: Color): void;
  fill_rect(rect: Rect2i, color: Color): void;
  get_used_rect(): Rect2i;
  get_region(region: Rect2i): Image;
  copy_from(src: Image): void;
  get_pixelv(point: Vector2i): Color;
  get_pixel(x: number, y: number): Color;
  set_pixelv(point: Vector2i, color: Color): void;
  set_pixel(x: number, y: number, color: Color): void;
  adjust_bcs(brightness: number, contrast: number, saturation: number): void;
  load_png_from_buffer(buffer: PackedByteArray): number;
  load_jpg_from_buffer(buffer: PackedByteArray): number;
  load_webp_from_buffer(buffer: PackedByteArray): number;
  load_tga_from_buffer(buffer: PackedByteArray): number;
  load_bmp_from_buffer(buffer: PackedByteArray): number;
  load_ktx_from_buffer(buffer: PackedByteArray): number;
  load_dds_from_buffer(buffer: PackedByteArray): number;
  load_svg_from_buffer(buffer: PackedByteArray, scale?: number): number;
  load_svg_from_string(svg_str: string, scale?: number): number;
}

export declare class ImageFormatLoader extends RefCounted {
}

export declare class ImageFormatLoaderExtension extends ImageFormatLoader {
  add_format_loader(): void;
  remove_format_loader(): void;
}

export declare class ImageTexture extends Texture2D {
  create_from_image(image: Image): ImageTexture;
  get_format(): number;
  set_image(image: Image): void;
  update(image: Image): void;
  set_size_override(size: Vector2i): void;
}

export declare class ImageTexture3D extends Texture3D {
  create(format: number, width: number, height: number, depth: number, use_mipmaps: boolean, data: any[]): number;
  update(data: any[]): void;
}

export declare class ImageTextureLayered extends TextureLayered {
  create_from_images(images: any[]): number;
  update_layer(image: Image, layer: number): void;
}

export declare class ImmediateMesh extends Mesh {
  surface_begin(primitive: number, material?: Material): void;
  surface_set_color(color: Color): void;
  surface_set_normal(normal: Vector3): void;
  surface_set_tangent(tangent: Plane): void;
  surface_set_uv(uv: Vector2): void;
  surface_set_uv2(uv2: Vector2): void;
  surface_add_vertex(vertex: Vector3): void;
  surface_add_vertex_2d(vertex: Vector2): void;
  surface_end(): void;
  clear_surfaces(): void;
}

export declare class ImporterMesh extends Resource {
  add_blend_shape(name: string): void;
  get_blend_shape_count(): number;
  get_blend_shape_name(blend_shape_idx: number): string;
  set_blend_shape_mode(mode: number): void;
  get_blend_shape_mode(): number;
  add_surface(primitive: number, arrays: any[], blend_shapes?: any[], lods?: Record<string, any>, material?: Material, name?: string, flags?: number): void;
  get_surface_count(): number;
  get_surface_primitive_type(surface_idx: number): number;
  get_surface_name(surface_idx: number): string;
  get_surface_arrays(surface_idx: number): any[];
  get_surface_blend_shape_arrays(surface_idx: number, blend_shape_idx: number): any[];
  get_surface_lod_count(surface_idx: number): number;
  get_surface_lod_size(surface_idx: number, lod_idx: number): number;
  get_surface_lod_indices(surface_idx: number, lod_idx: number): PackedInt32Array;
  get_surface_material(surface_idx: number): Material;
  get_surface_format(surface_idx: number): number;
  set_surface_name(surface_idx: number, name: string): void;
  set_surface_material(surface_idx: number, material: Material): void;
  generate_lods(normal_merge_angle: number, normal_split_angle: number, bone_transform_array: any[]): void;
  get_mesh(base_mesh?: ArrayMesh): ArrayMesh;
  clear(): void;
  set_lightmap_size_hint(size: Vector2i): void;
  get_lightmap_size_hint(): Vector2i;
}

export declare class ImporterMeshInstance3D extends Node3D {
  mesh: ImporterMesh;
  skin: Skin;
  skeleton_path: string;
  layer_mask: number;
  cast_shadow: number;
  visibility_range_begin: number;
  visibility_range_begin_margin: number;
  visibility_range_end: number;
  visibility_range_end_margin: number;
  visibility_range_fade_mode: number;
}

export declare class InputEvent extends Resource {
  device: number;
  is_action(action: string, exact_match?: boolean): boolean;
  is_action_pressed(action: string, allow_echo?: boolean, exact_match?: boolean): boolean;
  is_action_released(action: string, exact_match?: boolean): boolean;
  get_action_strength(action: string, exact_match?: boolean): number;
  is_canceled(): boolean;
  is_pressed(): boolean;
  is_released(): boolean;
  is_echo(): boolean;
  as_text(): string;
  is_match(event: InputEvent, exact_match?: boolean): boolean;
  is_action_type(): boolean;
  accumulate(with_event: InputEvent): boolean;
  xformed_by(xform: Transform2D, local_ofs?: Vector2): InputEvent;
}

export declare class InputEventAction extends InputEvent {
  action: string;
  pressed: boolean;
  strength: number;
  event_index: number;
}

export declare class InputEventFromWindow extends InputEvent {
  window_id: number;
}

export declare class InputEventGesture extends InputEventWithModifiers {
  position: Vector2;
}

export declare class InputEventJoypadButton extends InputEvent {
  button_index: number;
  pressure: number;
  pressed: boolean;
}

export declare class InputEventJoypadMotion extends InputEvent {
  axis: number;
  axis_value: number;
}

export declare class InputEventKey extends InputEventWithModifiers {
  pressed: boolean;
  keycode: number;
  physical_keycode: number;
  key_label: number;
  unicode: number;
  location: number;
  echo: boolean;
  get_keycode_with_modifiers(): number;
  get_physical_keycode_with_modifiers(): number;
  get_key_label_with_modifiers(): number;
  as_text_keycode(): string;
  as_text_physical_keycode(): string;
  as_text_key_label(): string;
  as_text_location(): string;
}

export declare class InputEventMIDI extends InputEvent {
  channel: number;
  message: number;
  pitch: number;
  velocity: number;
  instrument: number;
  pressure: number;
  controller_number: number;
  controller_value: number;
}

export declare class InputEventMagnifyGesture extends InputEventGesture {
  factor: number;
}

export declare class InputEventMouse extends InputEventWithModifiers {
  button_mask: number;
  position: Vector2;
  global_position: Vector2;
}

export declare class InputEventMouseButton extends InputEventMouse {
  factor: number;
  button_index: number;
  canceled: boolean;
  pressed: boolean;
  double_click: boolean;
}

export declare class InputEventMouseMotion extends InputEventMouse {
  tilt: Vector2;
  pressure: number;
  pen_inverted: boolean;
  relative: Vector2;
  screen_relative: Vector2;
  velocity: Vector2;
  screen_velocity: Vector2;
}

export declare class InputEventPanGesture extends InputEventGesture {
  delta: Vector2;
}

export declare class InputEventScreenDrag extends InputEventFromWindow {
  index: number;
  tilt: Vector2;
  pressure: number;
  pen_inverted: boolean;
  position: Vector2;
  relative: Vector2;
  screen_relative: Vector2;
  velocity: Vector2;
  screen_velocity: Vector2;
}

export declare class InputEventScreenTouch extends InputEventFromWindow {
  index: number;
  position: Vector2;
  canceled: boolean;
  pressed: boolean;
  double_tap: boolean;
}

export declare class InputEventShortcut extends InputEvent {
  shortcut: Shortcut;
}

export declare class InputEventWithModifiers extends InputEventFromWindow {
  command_or_control_autoremap: boolean;
  alt_pressed: boolean;
  shift_pressed: boolean;
  ctrl_pressed: boolean;
  meta_pressed: boolean;
  is_command_or_control_pressed(): boolean;
  get_modifiers_mask(): number;
}

export declare class InstancePlaceholder extends Node {
  get_stored_values(with_order?: boolean): Record<string, any>;
  create_instance(replace?: boolean, custom_scene?: PackedScene): Node;
  get_instance_path(): string;
}

export declare class IntervalTweener extends Tweener {
}

export declare class ItemList extends Control {
  select_mode: number;
  allow_reselect: boolean;
  allow_rmb_select: boolean;
  allow_search: boolean;
  max_text_lines: number;
  auto_width: boolean;
  auto_height: boolean;
  text_overrun_behavior: number;
  wraparound_items: boolean;
  item_count: number;
  max_columns: number;
  same_column_width: boolean;
  fixed_column_width: number;
  icon_mode: number;
  icon_scale: number;
  fixed_icon_size: Vector2i;
  add_item(text: string, icon?: Texture2D, selectable?: boolean): number;
  add_icon_item(icon: Texture2D, selectable?: boolean): number;
  set_item_text(idx: number, text: string): void;
  get_item_text(idx: number): string;
  set_item_icon(idx: number, icon: Texture2D): void;
  get_item_icon(idx: number): Texture2D;
  set_item_text_direction(idx: number, direction: number): void;
  get_item_text_direction(idx: number): number;
  set_item_language(idx: number, language: string): void;
  get_item_language(idx: number): string;
  set_item_auto_translate_mode(idx: number, mode: number): void;
  get_item_auto_translate_mode(idx: number): number;
  set_item_icon_transposed(idx: number, transposed: boolean): void;
  is_item_icon_transposed(idx: number): boolean;
  set_item_icon_region(idx: number, rect: Rect2): void;
  get_item_icon_region(idx: number): Rect2;
  set_item_icon_modulate(idx: number, modulate: Color): void;
  get_item_icon_modulate(idx: number): Color;
  set_item_selectable(idx: number, selectable: boolean): void;
  is_item_selectable(idx: number): boolean;
  set_item_disabled(idx: number, disabled: boolean): void;
  is_item_disabled(idx: number): boolean;
  set_item_metadata(idx: number, metadata: any): void;
  get_item_metadata(idx: number): any;
  set_item_custom_bg_color(idx: number, custom_bg_color: Color): void;
  get_item_custom_bg_color(idx: number): Color;
  set_item_custom_fg_color(idx: number, custom_fg_color: Color): void;
  get_item_custom_fg_color(idx: number): Color;
  get_item_rect(idx: number, expand?: boolean): Rect2;
  set_item_tooltip_enabled(idx: number, enable: boolean): void;
  is_item_tooltip_enabled(idx: number): boolean;
  set_item_tooltip(idx: number, tooltip: string): void;
  get_item_tooltip(idx: number): string;
  select(idx: number, single?: boolean): void;
  deselect(idx: number): void;
  deselect_all(): void;
  is_selected(idx: number): boolean;
  get_selected_items(): PackedInt32Array;
  move_item(from_idx: number, to_idx: number): void;
  remove_item(idx: number): void;
  clear(): void;
  sort_items_by_text(): void;
  is_anything_selected(): boolean;
  get_item_at_position(position: Vector2, exact?: boolean): number;
  ensure_current_is_visible(): void;
  get_v_scroll_bar(): VScrollBar;
  get_h_scroll_bar(): HScrollBar;
  force_update_list_size(): void;
}

export declare class JNISingleton extends Object {
}

export declare class JSONRPC extends Object {
  set_method(name: string, callback: Callable): void;
  process_action(action: any, recurse?: boolean): any;
  process_string(action: string): string;
  make_request(method: string, params: any, id: any): Record<string, any>;
  make_response(result: any, id: any): Record<string, any>;
  make_notification(method: string, params: any): Record<string, any>;
  make_response_error(code: number, message: string, id?: any): Record<string, any>;
}

export declare class JavaClass extends RefCounted {
  get_java_class_name(): string;
  get_java_method_list(): any[];
  get_java_parent_class(): JavaClass;
}

export declare class JavaObject extends RefCounted {
  get_java_class(): JavaClass;
}

export declare class JavaScriptObject extends RefCounted {
}

export declare class Joint2D extends Node2D {
  node_a: string;
  node_b: string;
  bias: number;
  disable_collision: boolean;
  get_rid(): RID;
}

export declare class Joint3D extends Node3D {
  node_a: string;
  node_b: string;
  solver_priority: number;
  exclude_nodes_from_collision: boolean;
  get_rid(): RID;
}

export declare class KinematicCollision2D extends RefCounted {
  get_position(): Vector2;
  get_normal(): Vector2;
  get_travel(): Vector2;
  get_remainder(): Vector2;
  get_angle(up_direction?: Vector2): number;
  get_depth(): number;
  get_local_shape(): any;
  get_collider(): any;
  get_collider_id(): number;
  get_collider_rid(): RID;
  get_collider_shape(): any;
  get_collider_shape_index(): number;
  get_collider_velocity(): Vector2;
}

export declare class KinematicCollision3D extends RefCounted {
  get_travel(): Vector3;
  get_remainder(): Vector3;
  get_depth(): number;
  get_collision_count(): number;
  get_position(collision_index?: number): Vector3;
  get_normal(collision_index?: number): Vector3;
  get_angle(collision_index?: number, up_direction?: Vector3): number;
  get_local_shape(collision_index?: number): any;
  get_collider(collision_index?: number): any;
  get_collider_id(collision_index?: number): number;
  get_collider_rid(collision_index?: number): RID;
  get_collider_shape(collision_index?: number): any;
  get_collider_shape_index(collision_index?: number): number;
  get_collider_velocity(collision_index?: number): Vector3;
}

export declare class Label extends Control {
  text: string;
  label_settings: LabelSettings;
  horizontal_alignment: number;
  vertical_alignment: number;
  autowrap_mode: number;
  autowrap_trim_flags: number;
  justification_flags: number;
  paragraph_separator: string;
  clip_text: boolean;
  text_overrun_behavior: number;
  ellipsis_char: string;
  uppercase: boolean;
  tab_stops: PackedFloat32Array;
  lines_skipped: number;
  max_lines_visible: number;
  visible_characters: number;
  visible_characters_behavior: number;
  visible_ratio: number;
  text_direction: number;
  language: string;
  structured_text_bidi_override: number;
  structured_text_bidi_override_options: any[];
  get_line_height(line?: number): number;
  get_line_count(): number;
  get_visible_line_count(): number;
  get_total_character_count(): number;
  get_character_bounds(pos: number): Rect2;
}

export declare class Label3D extends GeometryInstance3D {
  pixel_size: number;
  offset: Vector2;
  billboard: number;
  alpha_cut: number;
  alpha_scissor_threshold: number;
  alpha_hash_scale: number;
  alpha_antialiasing_mode: number;
  alpha_antialiasing_edge: number;
  texture_filter: number;
  render_priority: number;
  outline_render_priority: number;
  modulate: Color;
  outline_modulate: Color;
  text: string;
  font: Font;
  font_size: number;
  outline_size: number;
  horizontal_alignment: number;
  vertical_alignment: number;
  uppercase: boolean;
  line_spacing: number;
  autowrap_mode: number;
  autowrap_trim_flags: number;
  justification_flags: number;
  width: number;
  text_direction: number;
  language: string;
  structured_text_bidi_override: number;
  structured_text_bidi_override_options: any[];
  generate_triangle_mesh(): TriangleMesh;
}

export declare class LabelSettings extends Resource {
  line_spacing: number;
  paragraph_spacing: number;
  font: Font;
  font_size: number;
  font_color: Color;
  outline_size: number;
  outline_color: Color;
  shadow_size: number;
  shadow_color: Color;
  shadow_offset: Vector2;
  stacked_outline_count: number;
  stacked_shadow_count: number;
  add_stacked_outline(index?: number): void;
  move_stacked_outline(from_index: number, to_position: number): void;
  remove_stacked_outline(index: number): void;
  set_stacked_outline_size(index: number, size: number): void;
  get_stacked_outline_size(index: number): number;
  set_stacked_outline_color(index: number, color: Color): void;
  get_stacked_outline_color(index: number): Color;
  add_stacked_shadow(index?: number): void;
  move_stacked_shadow(from_index: number, to_position: number): void;
  remove_stacked_shadow(index: number): void;
  set_stacked_shadow_offset(index: number, offset: Vector2): void;
  get_stacked_shadow_offset(index: number): Vector2;
  set_stacked_shadow_color(index: number, color: Color): void;
  get_stacked_shadow_color(index: number): Color;
  set_stacked_shadow_outline_size(index: number, size: number): void;
  get_stacked_shadow_outline_size(index: number): number;
}

export declare class Light2D extends Node2D {
  enabled: boolean;
  editor_only: boolean;
  color: Color;
  energy: number;
  blend_mode: number;
  range_z_min: number;
  range_z_max: number;
  range_layer_min: number;
  range_layer_max: number;
  range_item_cull_mask: number;
  shadow_enabled: boolean;
  shadow_color: Color;
  shadow_filter: number;
  shadow_filter_smooth: number;
  shadow_item_cull_mask: number;
  set_height(height: number): void;
  get_height(): number;
}

export declare class Light3D extends VisualInstance3D {
  light_temperature: number;
  light_color: Color;
  light_negative: boolean;
  light_bake_mode: number;
  light_cull_mask: number;
  shadow_enabled: boolean;
  shadow_reverse_cull_face: boolean;
  shadow_caster_mask: number;
  distance_fade_enabled: boolean;
  distance_fade_begin: number;
  distance_fade_shadow: number;
  distance_fade_length: number;
  editor_only: boolean;
  get_correlated_color(): Color;
}

export declare class LightOccluder2D extends Node2D {
  occluder: OccluderPolygon2D;
  sdf_collision: boolean;
  occluder_light_mask: number;
}

export declare class LightmapGI extends VisualInstance3D {
  quality: number;
  supersampling: boolean;
  supersampling_factor: number;
  bounces: number;
  bounce_indirect_energy: number;
  directional: boolean;
  shadowmask_mode: number;
  use_texture_for_bounces: boolean;
  interior: boolean;
  use_denoiser: boolean;
  denoiser_strength: number;
  denoiser_range: number;
  bias: number;
  texel_scale: number;
  max_texture_size: number;
  environment_mode: number;
  environment_custom_sky: Sky;
  environment_custom_color: Color;
  environment_custom_energy: number;
  generate_probes_subdiv: number;
  light_data: LightmapGIData;
}

export declare class LightmapGIData extends Resource {
  lightmap_textures: any[];
  shadowmask_textures: any[];
  uses_spherical_harmonics: boolean;
  light_texture: TextureLayered;
  add_user(path: string, uv_scale: Rect2, slice_index: number, sub_instance: number): void;
  get_user_count(): number;
  get_user_path(user_idx: number): string;
  clear_users(): void;
}

export declare class LightmapProbe extends Node3D {
}

export declare class Lightmapper extends RefCounted {
}

export declare class LightmapperRD extends Lightmapper {
}

export declare class Line2D extends Node2D {
  points: PackedVector2Array;
  closed: boolean;
  width: number;
  width_curve: Curve;
  default_color: Color;
  gradient: Gradient;
  texture: Texture2D;
  texture_mode: number;
  joint_mode: number;
  begin_cap_mode: number;
  end_cap_mode: number;
  sharp_limit: number;
  round_precision: number;
  antialiased: boolean;
  set_point_position(index: number, position: Vector2): void;
  get_point_position(index: number): Vector2;
  get_point_count(): number;
  add_point(position: Vector2, index?: number): void;
  remove_point(index: number): void;
  clear_points(): void;
}

export declare class LineEdit extends Control {
  text: string;
  placeholder_text: string;
  alignment: number;
  max_length: number;
  editable: boolean;
  keep_editing_on_text_submit: boolean;
  expand_to_text_length: boolean;
  context_menu_enabled: boolean;
  emoji_menu_enabled: boolean;
  backspace_deletes_composite_character_enabled: boolean;
  virtual_keyboard_enabled: boolean;
  virtual_keyboard_show_on_focus: boolean;
  virtual_keyboard_type: number;
  clear_button_enabled: boolean;
  shortcut_keys_enabled: boolean;
  middle_mouse_paste_enabled: boolean;
  selecting_enabled: boolean;
  deselect_on_focus_loss_enabled: boolean;
  drag_and_drop_selection_enabled: boolean;
  right_icon: Texture2D;
  flat: boolean;
  draw_control_chars: boolean;
  select_all_on_focus: boolean;
  caret_blink: boolean;
  caret_blink_interval: number;
  caret_column: number;
  caret_force_displayed: boolean;
  caret_mid_grapheme: boolean;
  secret: boolean;
  secret_character: string;
  text_direction: number;
  language: string;
  structured_text_bidi_override: number;
  structured_text_bidi_override_options: any[];
  has_ime_text(): boolean;
  cancel_ime(): void;
  apply_ime(): void;
  edit(): void;
  unedit(): void;
  is_editing(): boolean;
  clear(): void;
  select(from?: number, to?: number): void;
  select_all(): void;
  deselect(): void;
  has_undo(): boolean;
  has_redo(): boolean;
  has_selection(): boolean;
  get_selected_text(): string;
  get_selection_from_column(): number;
  get_selection_to_column(): number;
  get_next_composite_character_column(column: number): number;
  get_previous_composite_character_column(column: number): number;
  get_scroll_offset(): number;
  insert_text_at_caret(text: string): void;
  delete_char_at_caret(): void;
  delete_text(from_column: number, to_column: number): void;
  menu_option(option: number): void;
  get_menu(): PopupMenu;
  is_menu_visible(): boolean;
}

export declare class LinkButton extends BaseButton {
  text: string;
  underline: number;
  uri: string;
  text_direction: number;
  language: string;
  structured_text_bidi_override: number;
  structured_text_bidi_override_options: any[];
}

export declare class Logger extends RefCounted {
}

export declare class LookAtModifier3D extends SkeletonModifier3D {
  target_node: string;
  bone_name: string;
  bone: number;
  forward_axis: number;
  primary_rotation_axis: number;
  use_secondary_rotation: boolean;
  origin_from: number;
  origin_bone_name: string;
  origin_bone: number;
  origin_external_node: string;
  origin_offset: Vector3;
  origin_safe_margin: number;
  duration: number;
  transition_type: number;
  ease_type: number;
  use_angle_limitation: boolean;
  symmetry_limitation: boolean;
  primary_limit_angle: number;
  primary_damp_threshold: number;
  primary_positive_limit_angle: number;
  primary_positive_damp_threshold: number;
  primary_negative_limit_angle: number;
  primary_negative_damp_threshold: number;
  secondary_limit_angle: number;
  secondary_damp_threshold: number;
  secondary_positive_limit_angle: number;
  secondary_positive_damp_threshold: number;
  secondary_negative_limit_angle: number;
  secondary_negative_damp_threshold: number;
  get_interpolation_remaining(): number;
  is_interpolating(): boolean;
  is_target_within_limitation(): boolean;
}

export declare class MainLoop extends Object {
}

export declare class MarginContainer extends Container {
}

export declare class Marker2D extends Node2D {
  gizmo_extents: number;
}

export declare class Marker3D extends Node3D {
  gizmo_extents: number;
}

export declare class Material extends Resource {
  render_priority: number;
  next_pass: Material;
  inspect_native_shader_code(): void;
  create_placeholder(): Resource;
}

export declare class MenuBar extends Control {
  flat: boolean;
  start_index: number;
  switch_on_hover: boolean;
  prefer_global_menu: boolean;
  text_direction: number;
  language: string;
  set_disable_shortcuts(disabled: boolean): void;
  is_native_menu(): boolean;
  get_menu_count(): number;
  set_menu_title(menu: number, title: string): void;
  get_menu_title(menu: number): string;
  set_menu_tooltip(menu: number, tooltip: string): void;
  get_menu_tooltip(menu: number): string;
  set_menu_disabled(menu: number, disabled: boolean): void;
  is_menu_disabled(menu: number): boolean;
  set_menu_hidden(menu: number, hidden: boolean): void;
  is_menu_hidden(menu: number): boolean;
  get_menu_popup(menu: number): PopupMenu;
}

export declare class MenuButton extends Button {
  switch_on_hover: boolean;
  item_count: number;
  get_popup(): PopupMenu;
  show_popup(): void;
  set_disable_shortcuts(disabled: boolean): void;
}

export declare class Mesh extends Resource {
  lightmap_size_hint: Vector2i;
  get_aabb(): AABB;
  get_faces(): PackedVector3Array;
  get_surface_count(): number;
  surface_get_arrays(surf_idx: number): any[];
  surface_get_blend_shape_arrays(surf_idx: number): any[];
  surface_set_material(surf_idx: number, material: Material): void;
  surface_get_material(surf_idx: number): Material;
  create_placeholder(): Resource;
  create_trimesh_shape(): ConcavePolygonShape3D;
  create_convex_shape(clean?: boolean, simplify?: boolean): ConvexPolygonShape3D;
  create_outline(margin: number): Mesh;
  generate_triangle_mesh(): TriangleMesh;
}

export declare class MeshConvexDecompositionSettings extends RefCounted {
  max_concavity: number;
  symmetry_planes_clipping_bias: number;
  revolution_axes_clipping_bias: number;
  min_volume_per_convex_hull: number;
  resolution: number;
  max_num_vertices_per_convex_hull: number;
  plane_downsampling: number;
  convex_hull_downsampling: number;
  normalize_mesh: boolean;
  mode: number;
  convex_hull_approximation: boolean;
  max_convex_hulls: number;
  project_hull_vertices: boolean;
}

export declare class MeshDataTool extends RefCounted {
  clear(): void;
  create_from_surface(mesh: ArrayMesh, surface: number): number;
  commit_to_surface(mesh: ArrayMesh, compression_flags?: number): number;
  get_format(): number;
  get_vertex_count(): number;
  get_edge_count(): number;
  get_face_count(): number;
  set_vertex(idx: number, vertex: Vector3): void;
  get_vertex(idx: number): Vector3;
  set_vertex_normal(idx: number, normal: Vector3): void;
  get_vertex_normal(idx: number): Vector3;
  set_vertex_tangent(idx: number, tangent: Plane): void;
  get_vertex_tangent(idx: number): Plane;
  set_vertex_uv(idx: number, uv: Vector2): void;
  get_vertex_uv(idx: number): Vector2;
  set_vertex_uv2(idx: number, uv2: Vector2): void;
  get_vertex_uv2(idx: number): Vector2;
  set_vertex_color(idx: number, color: Color): void;
  get_vertex_color(idx: number): Color;
  set_vertex_bones(idx: number, bones: PackedInt32Array): void;
  get_vertex_bones(idx: number): PackedInt32Array;
  set_vertex_weights(idx: number, weights: PackedFloat32Array): void;
  get_vertex_weights(idx: number): PackedFloat32Array;
  set_vertex_meta(idx: number, meta: any): void;
  get_vertex_meta(idx: number): any;
  get_vertex_edges(idx: number): PackedInt32Array;
  get_vertex_faces(idx: number): PackedInt32Array;
  get_edge_vertex(idx: number, vertex: number): number;
  get_edge_faces(idx: number): PackedInt32Array;
  set_edge_meta(idx: number, meta: any): void;
  get_edge_meta(idx: number): any;
  get_face_vertex(idx: number, vertex: number): number;
  get_face_edge(idx: number, edge: number): number;
  set_face_meta(idx: number, meta: any): void;
  get_face_meta(idx: number): any;
  get_face_normal(idx: number): Vector3;
  set_material(material: Material): void;
  get_material(): Material;
}

export declare class MeshInstance2D extends Node2D {
  mesh: Mesh;
  texture: Texture2D;
}

export declare class MeshInstance3D extends GeometryInstance3D {
  mesh: Mesh;
  skin: Skin;
  skeleton: string;
  get_skin_reference(): SkinReference;
  get_surface_override_material_count(): number;
  set_surface_override_material(surface: number, material: Material): void;
  get_surface_override_material(surface: number): Material;
  get_active_material(surface: number): Material;
  create_trimesh_collision(): void;
  create_convex_collision(clean?: boolean, simplify?: boolean): void;
  create_multiple_convex_collisions(settings?: MeshConvexDecompositionSettings): void;
  get_blend_shape_count(): number;
  find_blend_shape_by_name(name: string): number;
  get_blend_shape_value(blend_shape_idx: number): number;
  set_blend_shape_value(blend_shape_idx: number, value: number): void;
  create_debug_tangents(): void;
  bake_mesh_from_current_blend_shape_mix(existing?: ArrayMesh): ArrayMesh;
  bake_mesh_from_current_skeleton_pose(existing?: ArrayMesh): ArrayMesh;
}

export declare class MeshLibrary extends Resource {
  create_item(id: number): void;
  set_item_name(id: number, name: string): void;
  set_item_mesh(id: number, mesh: Mesh): void;
  set_item_mesh_transform(id: number, mesh_transform: Transform3D): void;
  set_item_mesh_cast_shadow(id: number, shadow_casting_setting: number): void;
  set_item_navigation_mesh(id: number, navigation_mesh: NavigationMesh): void;
  set_item_navigation_mesh_transform(id: number, navigation_mesh: Transform3D): void;
  set_item_navigation_layers(id: number, navigation_layers: number): void;
  set_item_shapes(id: number, shapes: any[]): void;
  set_item_preview(id: number, texture: Texture2D): void;
  get_item_name(id: number): string;
  get_item_mesh(id: number): Mesh;
  get_item_mesh_transform(id: number): Transform3D;
  get_item_mesh_cast_shadow(id: number): number;
  get_item_navigation_mesh(id: number): NavigationMesh;
  get_item_navigation_mesh_transform(id: number): Transform3D;
  get_item_navigation_layers(id: number): number;
  get_item_shapes(id: number): any[];
  get_item_preview(id: number): Texture2D;
  remove_item(id: number): void;
  find_item_by_name(name: string): number;
  clear(): void;
  get_item_list(): PackedInt32Array;
  get_last_unused_item_id(): number;
}

export declare class MeshTexture extends Texture2D {
  mesh: Mesh;
  base_texture: Texture2D;
  image_size: Vector2;
}

export declare class MethodTweener extends Tweener {
  set_delay(delay: number): MethodTweener;
  set_trans(trans: number): MethodTweener;
  set_ease(ease: number): MethodTweener;
}

export declare class MissingNode extends Node {
  original_class: string;
  original_scene: string;
  recording_properties: boolean;
}

export declare class MissingResource extends Resource {
  original_class: string;
  recording_properties: boolean;
}

export declare class MobileVRInterface extends XRInterface {
  eye_height: number;
  iod: number;
  display_width: number;
  display_to_lens: number;
  offset_rect: Rect2;
  oversample: number;
  k1: number;
  k2: number;
  vrs_min_radius: number;
  vrs_strength: number;
}

export declare class ModifierBoneTarget3D extends SkeletonModifier3D {
  bone_name: string;
  bone: number;
}

export declare class MovieWriter extends Object {
  add_writer(writer: MovieWriter): void;
}

export declare class MultiMesh extends Resource {
  transform_format: number;
  use_colors: boolean;
  use_custom_data: boolean;
  custom_aabb: AABB;
  instance_count: number;
  visible_instance_count: number;
  mesh: Mesh;
  buffer: PackedFloat32Array;
  physics_interpolation_quality: number;
  set_instance_transform(instance: number, transform: Transform3D): void;
  set_instance_transform_2d(instance: number, transform: Transform2D): void;
  get_instance_transform(instance: number): Transform3D;
  get_instance_transform_2d(instance: number): Transform2D;
  set_instance_color(instance: number, color: Color): void;
  get_instance_color(instance: number): Color;
  set_instance_custom_data(instance: number, custom_data: Color): void;
  get_instance_custom_data(instance: number): Color;
  reset_instance_physics_interpolation(instance: number): void;
  get_aabb(): AABB;
  set_buffer_interpolated(buffer_curr: PackedFloat32Array, buffer_prev: PackedFloat32Array): void;
}

export declare class MultiMeshInstance2D extends Node2D {
  multimesh: MultiMesh;
  texture: Texture2D;
}

export declare class MultiMeshInstance3D extends GeometryInstance3D {
  multimesh: MultiMesh;
}

export declare class MultiplayerAPIExtension extends MultiplayerAPI {
}

export declare class MultiplayerPeerExtension extends MultiplayerPeer {
}

export declare class MultiplayerSpawner extends Node {
  spawn_path: string;
  spawn_limit: number;
  spawn_function: Callable;
  add_spawnable_scene(path: string): void;
  get_spawnable_scene_count(): number;
  get_spawnable_scene(index: number): string;
  clear_spawnable_scenes(): void;
  spawn(data?: any): Node;
}

export declare class MultiplayerSynchronizer extends Node {
  root_path: string;
  replication_interval: number;
  delta_interval: number;
  replication_config: SceneReplicationConfig;
  visibility_update_mode: number;
  public_visibility: boolean;
  update_visibility(for_peer?: number): void;
  add_visibility_filter(filter: Callable): void;
  remove_visibility_filter(filter: Callable): void;
  set_visibility_for(peer: number, visible: boolean): void;
  get_visibility_for(peer: number): boolean;
}

export declare class NavigationAgent2D extends Node {
  target_position: Vector2;
  path_desired_distance: number;
  target_desired_distance: number;
  path_max_distance: number;
  navigation_layers: number;
  pathfinding_algorithm: number;
  path_postprocessing: number;
  path_metadata_flags: number;
  simplify_path: boolean;
  simplify_epsilon: number;
  path_return_max_length: number;
  path_return_max_radius: number;
  path_search_max_polygons: number;
  path_search_max_distance: number;
  avoidance_enabled: boolean;
  velocity: Vector2;
  radius: number;
  neighbor_distance: number;
  max_neighbors: number;
  time_horizon_agents: number;
  time_horizon_obstacles: number;
  max_speed: number;
  avoidance_layers: number;
  avoidance_mask: number;
  avoidance_priority: number;
  debug_enabled: boolean;
  debug_use_custom: boolean;
  debug_path_custom_color: Color;
  debug_path_custom_point_size: number;
  debug_path_custom_line_width: number;
  get_rid(): RID;
  set_navigation_layer_value(layer_number: number, value: boolean): void;
  get_navigation_layer_value(layer_number: number): boolean;
  set_navigation_map(navigation_map: RID): void;
  get_navigation_map(): RID;
  get_path_length(): number;
  get_next_path_position(): Vector2;
  set_velocity_forced(velocity: Vector2): void;
  distance_to_target(): number;
  get_current_navigation_result(): NavigationPathQueryResult2D;
  get_current_navigation_path(): PackedVector2Array;
  get_current_navigation_path_index(): number;
  is_target_reached(): boolean;
  is_target_reachable(): boolean;
  is_navigation_finished(): boolean;
  get_final_position(): Vector2;
  set_avoidance_layer_value(layer_number: number, value: boolean): void;
  get_avoidance_layer_value(layer_number: number): boolean;
  set_avoidance_mask_value(mask_number: number, value: boolean): void;
  get_avoidance_mask_value(mask_number: number): boolean;
}

export declare class NavigationAgent3D extends Node {
  target_position: Vector3;
  path_desired_distance: number;
  target_desired_distance: number;
  path_height_offset: number;
  path_max_distance: number;
  navigation_layers: number;
  pathfinding_algorithm: number;
  path_postprocessing: number;
  path_metadata_flags: number;
  simplify_path: boolean;
  simplify_epsilon: number;
  path_return_max_length: number;
  path_return_max_radius: number;
  path_search_max_polygons: number;
  path_search_max_distance: number;
  avoidance_enabled: boolean;
  velocity: Vector3;
  height: number;
  radius: number;
  neighbor_distance: number;
  max_neighbors: number;
  time_horizon_agents: number;
  time_horizon_obstacles: number;
  max_speed: number;
  use_3d_avoidance: boolean;
  keep_y_velocity: boolean;
  avoidance_layers: number;
  avoidance_mask: number;
  avoidance_priority: number;
  debug_enabled: boolean;
  debug_use_custom: boolean;
  debug_path_custom_color: Color;
  debug_path_custom_point_size: number;
  get_rid(): RID;
  set_navigation_layer_value(layer_number: number, value: boolean): void;
  get_navigation_layer_value(layer_number: number): boolean;
  set_navigation_map(navigation_map: RID): void;
  get_navigation_map(): RID;
  get_path_length(): number;
  get_next_path_position(): Vector3;
  set_velocity_forced(velocity: Vector3): void;
  distance_to_target(): number;
  get_current_navigation_result(): NavigationPathQueryResult3D;
  get_current_navigation_path(): PackedVector3Array;
  get_current_navigation_path_index(): number;
  is_target_reached(): boolean;
  is_target_reachable(): boolean;
  is_navigation_finished(): boolean;
  get_final_position(): Vector3;
  set_avoidance_layer_value(layer_number: number, value: boolean): void;
  get_avoidance_layer_value(layer_number: number): boolean;
  set_avoidance_mask_value(mask_number: number, value: boolean): void;
  get_avoidance_mask_value(mask_number: number): boolean;
}

export declare class NavigationLink2D extends Node2D {
  enabled: boolean;
  bidirectional: boolean;
  navigation_layers: number;
  start_position: Vector2;
  end_position: Vector2;
  enter_cost: number;
  travel_cost: number;
  get_rid(): RID;
  set_navigation_map(navigation_map: RID): void;
  get_navigation_map(): RID;
  set_navigation_layer_value(layer_number: number, value: boolean): void;
  get_navigation_layer_value(layer_number: number): boolean;
  set_global_start_position(position: Vector2): void;
  get_global_start_position(): Vector2;
  set_global_end_position(position: Vector2): void;
  get_global_end_position(): Vector2;
}

export declare class NavigationLink3D extends Node3D {
  enabled: boolean;
  bidirectional: boolean;
  navigation_layers: number;
  start_position: Vector3;
  end_position: Vector3;
  enter_cost: number;
  travel_cost: number;
  get_rid(): RID;
  set_navigation_map(navigation_map: RID): void;
  get_navigation_map(): RID;
  set_navigation_layer_value(layer_number: number, value: boolean): void;
  get_navigation_layer_value(layer_number: number): boolean;
  set_global_start_position(position: Vector3): void;
  get_global_start_position(): Vector3;
  set_global_end_position(position: Vector3): void;
  get_global_end_position(): Vector3;
}

export declare class NavigationMesh extends Resource {
  vertices: PackedVector3Array;
  sample_partition_type: number;
  geometry_parsed_geometry_type: number;
  geometry_collision_mask: number;
  geometry_source_geometry_mode: number;
  cell_size: number;
  cell_height: number;
  border_size: number;
  agent_height: number;
  agent_radius: number;
  agent_max_climb: number;
  agent_max_slope: number;
  region_min_size: number;
  region_merge_size: number;
  edge_max_length: number;
  edge_max_error: number;
  vertices_per_polygon: number;
  detail_sample_distance: number;
  detail_sample_max_error: number;
  filter_low_hanging_obstacles: boolean;
  filter_ledge_spans: boolean;
  filter_walkable_low_height_spans: boolean;
  filter_baking_aabb: AABB;
  filter_baking_aabb_offset: Vector3;
  set_collision_mask_value(layer_number: number, value: boolean): void;
  get_collision_mask_value(layer_number: number): boolean;
  add_polygon(polygon: PackedInt32Array): void;
  get_polygon_count(): number;
  get_polygon(idx: number): PackedInt32Array;
  clear_polygons(): void;
  create_from_mesh(mesh: Mesh): void;
  clear(): void;
}

export declare class NavigationMeshSourceGeometryData2D extends Resource {
  projected_obstructions: any[];
  clear(): void;
  has_data(): boolean;
  append_traversable_outlines(traversable_outlines: any[]): void;
  append_obstruction_outlines(obstruction_outlines: any[]): void;
  add_traversable_outline(shape_outline: PackedVector2Array): void;
  add_obstruction_outline(shape_outline: PackedVector2Array): void;
  merge(other_geometry: NavigationMeshSourceGeometryData2D): void;
  add_projected_obstruction(vertices: PackedVector2Array, carve: boolean): void;
  clear_projected_obstructions(): void;
  get_bounds(): Rect2;
}

export declare class NavigationMeshSourceGeometryData3D extends Resource {
  indices: PackedInt32Array;
  projected_obstructions: any[];
  append_arrays(vertices: PackedFloat32Array, indices: PackedInt32Array): void;
  clear(): void;
  has_data(): boolean;
  add_mesh(mesh: Mesh, xform: Transform3D): void;
  add_mesh_array(mesh_array: any[], xform: Transform3D): void;
  add_faces(faces: PackedVector3Array, xform: Transform3D): void;
  merge(other_geometry: NavigationMeshSourceGeometryData3D): void;
  add_projected_obstruction(vertices: PackedVector3Array, elevation: number, height: number, carve: boolean): void;
  clear_projected_obstructions(): void;
  get_bounds(): AABB;
}

export declare class NavigationObstacle2D extends Node2D {
  radius: number;
  vertices: PackedVector2Array;
  affect_navigation_mesh: boolean;
  carve_navigation_mesh: boolean;
  avoidance_enabled: boolean;
  velocity: Vector2;
  avoidance_layers: number;
  get_rid(): RID;
  set_navigation_map(navigation_map: RID): void;
  get_navigation_map(): RID;
  set_avoidance_layer_value(layer_number: number, value: boolean): void;
  get_avoidance_layer_value(layer_number: number): boolean;
}

export declare class NavigationObstacle3D extends Node3D {
  radius: number;
  height: number;
  vertices: PackedVector3Array;
  affect_navigation_mesh: boolean;
  carve_navigation_mesh: boolean;
  avoidance_enabled: boolean;
  velocity: Vector3;
  avoidance_layers: number;
  use_3d_avoidance: boolean;
  get_rid(): RID;
  set_navigation_map(navigation_map: RID): void;
  get_navigation_map(): RID;
  set_avoidance_layer_value(layer_number: number, value: boolean): void;
  get_avoidance_layer_value(layer_number: number): boolean;
}

export declare class NavigationPathQueryParameters2D extends RefCounted {
  map: RID;
  start_position: Vector2;
  target_position: Vector2;
  navigation_layers: number;
  pathfinding_algorithm: number;
  path_postprocessing: number;
  metadata_flags: number;
  simplify_path: boolean;
  simplify_epsilon: number;
  excluded_regions: any[];
  included_regions: any[];
  path_return_max_length: number;
  path_return_max_radius: number;
  path_search_max_polygons: number;
  path_search_max_distance: number;
}

export declare class NavigationPathQueryParameters3D extends RefCounted {
  map: RID;
  start_position: Vector3;
  target_position: Vector3;
  navigation_layers: number;
  pathfinding_algorithm: number;
  path_postprocessing: number;
  metadata_flags: number;
  simplify_path: boolean;
  simplify_epsilon: number;
  excluded_regions: any[];
  included_regions: any[];
  path_return_max_length: number;
  path_return_max_radius: number;
  path_search_max_polygons: number;
  path_search_max_distance: number;
}

export declare class NavigationPathQueryResult2D extends RefCounted {
  path: PackedVector2Array;
  path_types: PackedInt32Array;
  path_rids: any[];
  path_owner_ids: PackedInt64Array;
  path_length: number;
  reset(): void;
}

export declare class NavigationPathQueryResult3D extends RefCounted {
  path: PackedVector3Array;
  path_types: PackedInt32Array;
  path_rids: any[];
  path_owner_ids: PackedInt64Array;
  path_length: number;
  reset(): void;
}

export declare class NavigationPolygon extends Resource {
  vertices: PackedVector2Array;
  sample_partition_type: number;
  parsed_geometry_type: number;
  parsed_collision_mask: number;
  source_geometry_mode: number;
  cell_size: number;
  border_size: number;
  agent_radius: number;
  baking_rect: Rect2;
  baking_rect_offset: Vector2;
  add_polygon(polygon: PackedInt32Array): void;
  get_polygon_count(): number;
  get_polygon(idx: number): PackedInt32Array;
  clear_polygons(): void;
  get_navigation_mesh(): NavigationMesh;
  add_outline(outline: PackedVector2Array): void;
  add_outline_at_index(outline: PackedVector2Array, index: number): void;
  get_outline_count(): number;
  set_outline(idx: number, outline: PackedVector2Array): void;
  get_outline(idx: number): PackedVector2Array;
  remove_outline(idx: number): void;
  clear_outlines(): void;
  make_polygons_from_outlines(): void;
  set_parsed_collision_mask_value(layer_number: number, value: boolean): void;
  get_parsed_collision_mask_value(layer_number: number): boolean;
  clear(): void;
}

export declare class NavigationRegion2D extends Node2D {
  navigation_polygon: NavigationPolygon;
  enabled: boolean;
  use_edge_connections: boolean;
  navigation_layers: number;
  enter_cost: number;
  travel_cost: number;
  get_rid(): RID;
  set_navigation_map(navigation_map: RID): void;
  get_navigation_map(): RID;
  set_navigation_layer_value(layer_number: number, value: boolean): void;
  get_navigation_layer_value(layer_number: number): boolean;
  get_region_rid(): RID;
  bake_navigation_polygon(on_thread?: boolean): void;
  is_baking(): boolean;
  get_bounds(): Rect2;
}

export declare class NavigationRegion3D extends Node3D {
  navigation_mesh: NavigationMesh;
  enabled: boolean;
  use_edge_connections: boolean;
  navigation_layers: number;
  enter_cost: number;
  travel_cost: number;
  get_rid(): RID;
  set_navigation_map(navigation_map: RID): void;
  get_navigation_map(): RID;
  set_navigation_layer_value(layer_number: number, value: boolean): void;
  get_navigation_layer_value(layer_number: number): boolean;
  get_region_rid(): RID;
  bake_navigation_mesh(on_thread?: boolean): void;
  is_baking(): boolean;
  get_bounds(): AABB;
}

export declare class NinePatchRect extends Control {
  texture: Texture2D;
  draw_center: boolean;
  region_rect: Rect2;
  axis_stretch_horizontal: number;
  axis_stretch_vertical: number;
}

export declare class Node extends Object {
  name: string;
  unique_name_in_owner: boolean;
  scene_file_path: string;
  owner: Node;
  readonly multiplayer: MultiplayerAPI;
  process_mode: number;
  process_priority: number;
  process_physics_priority: number;
  process_thread_group_order: number;
  process_thread_messages: number;
  physics_interpolation_mode: number;
  auto_translate_mode: number;
  editor_description: string;
  print_orphan_nodes(): void;
  get_orphan_node_ids(): any[];
  add_sibling(sibling: Node, force_readable_name?: boolean): void;
  add_child(node: Node, force_readable_name?: boolean, internal?: number): void;
  remove_child(node: Node): void;
  reparent(new_parent: Node, keep_global_transform?: boolean): void;
  get_child_count(include_internal?: boolean): number;
  get_children(include_internal?: boolean): any[];
  get_child(idx: number, include_internal?: boolean): Node;
  has_node(path: string): boolean;
  get_node(path: string): Node;
  get_node_or_null(path: string): Node;
  get_parent(): Node;
  find_child(pattern: string, recursive?: boolean, owned?: boolean): Node;
  find_children(pattern: string, type?: string, recursive?: boolean, owned?: boolean): any[];
  find_parent(pattern: string): Node;
  has_node_and_resource(path: string): boolean;
  get_node_and_resource(path: string): any[];
  is_inside_tree(): boolean;
  is_part_of_edited_scene(): boolean;
  is_ancestor_of(node: Node): boolean;
  is_greater_than(node: Node): boolean;
  get_path(): string;
  get_path_to(node: Node, use_unique_path?: boolean): string;
  add_to_group(group: string, persistent?: boolean): void;
  remove_from_group(group: string): void;
  is_in_group(group: string): boolean;
  move_child(child_node: Node, to_index: number): void;
  get_groups(): any[];
  get_index(include_internal?: boolean): number;
  print_tree(): void;
  print_tree_pretty(): void;
  get_tree_string(): string;
  get_tree_string_pretty(): string;
  propagate_notification(what: number): void;
  set_physics_process(enable: boolean): void;
  get_physics_process_delta_time(): number;
  is_physics_processing(): boolean;
  get_process_delta_time(): number;
  set_process(enable: boolean): void;
  is_processing(): boolean;
  set_process_input(enable: boolean): void;
  is_processing_input(): boolean;
  set_process_shortcut_input(enable: boolean): void;
  is_processing_shortcut_input(): boolean;
  set_process_unhandled_input(enable: boolean): void;
  is_processing_unhandled_input(): boolean;
  set_process_unhandled_key_input(enable: boolean): void;
  is_processing_unhandled_key_input(): boolean;
  can_process(): boolean;
  queue_accessibility_update(): void;
  get_accessibility_element(): RID;
  set_display_folded(fold: boolean): void;
  is_displayed_folded(): boolean;
  set_process_internal(enable: boolean): void;
  is_processing_internal(): boolean;
  set_physics_process_internal(enable: boolean): void;
  is_physics_processing_internal(): boolean;
  is_physics_interpolated(): boolean;
  is_physics_interpolated_and_enabled(): boolean;
  reset_physics_interpolation(): void;
  can_auto_translate(): boolean;
  set_translation_domain_inherited(): void;
  get_window(): Window;
  get_last_exclusive_window(): Window;
  get_tree(): SceneTree;
  create_tween(): Tween;
  duplicate(flags?: number): Node;
  replace_by(node: Node, keep_groups?: boolean): void;
  set_scene_instance_load_placeholder(load_placeholder: boolean): void;
  get_scene_instance_load_placeholder(): boolean;
  set_editable_instance(node: Node, is_editable: boolean): void;
  is_editable_instance(node: Node): boolean;
  get_viewport(): Viewport;
  queue_free(): void;
  request_ready(): void;
  is_node_ready(): boolean;
  set_multiplayer_authority(id: number, recursive?: boolean): void;
  get_multiplayer_authority(): number;
  is_multiplayer_authority(): boolean;
  rpc_config(method: string, config: any): void;
  get_node_rpc_config(): any;
  atr(message: string, context?: string): string;
  atr_n(message: string, plural_message: string, n: number, context?: string): string;
  update_configuration_warnings(): void;
  set_deferred_thread_group(property: string, value: any): void;
  notify_deferred_thread_group(what: number): void;
  set_thread_safe(property: string, value: any): void;
  notify_thread_safe(what: number): void;
}

export declare class Node2D extends CanvasItem {
  position: Vector2;
  rotation: number;
  rotation_degrees: number;
  scale: Vector2;
  skew: number;
  transform: Transform2D;
  global_position: Vector2;
  global_rotation: number;
  global_rotation_degrees: number;
  global_scale: Vector2;
  global_skew: number;
  global_transform: Transform2D;
  rotate(radians: number): void;
  move_local_x(delta: number, scaled?: boolean): void;
  move_local_y(delta: number, scaled?: boolean): void;
  translate(offset: Vector2): void;
  global_translate(offset: Vector2): void;
  apply_scale(ratio: Vector2): void;
  look_at(point: Vector2): void;
  get_angle_to(point: Vector2): number;
  to_local(global_point: Vector2): Vector2;
  to_global(local_point: Vector2): Vector2;
  get_relative_transform_to_parent(parent: Node): Transform2D;
}

export declare class Node3D extends Node {
  transform: Transform3D;
  global_transform: Transform3D;
  position: Vector3;
  rotation: Vector3;
  rotation_degrees: Vector3;
  quaternion: Quaternion;
  basis: Basis;
  scale: Vector3;
  rotation_edit_mode: number;
  rotation_order: number;
  top_level: boolean;
  global_position: Vector3;
  global_basis: Basis;
  global_rotation: Vector3;
  global_rotation_degrees: Vector3;
  visible: boolean;
  visibility_parent: string;
  get_global_transform_interpolated(): Transform3D;
  get_parent_node_3d(): Node3D;
  set_ignore_transform_notification(enabled: boolean): void;
  set_disable_scale(disable: boolean): void;
  is_scale_disabled(): boolean;
  get_world_3d(): World3D;
  force_update_transform(): void;
  update_gizmos(): void;
  add_gizmo(gizmo: Node3DGizmo): void;
  get_gizmos(): any[];
  clear_gizmos(): void;
  set_subgizmo_selection(gizmo: Node3DGizmo, id: number, transform: Transform3D): void;
  clear_subgizmo_selection(): void;
  is_visible_in_tree(): boolean;
  show(): void;
  hide(): void;
  set_notify_local_transform(enable: boolean): void;
  is_local_transform_notification_enabled(): boolean;
  set_notify_transform(enable: boolean): void;
  is_transform_notification_enabled(): boolean;
  rotate(axis: Vector3, angle: number): void;
  global_rotate(axis: Vector3, angle: number): void;
  global_scale(scale: Vector3): void;
  global_translate(offset: Vector3): void;
  rotate_object_local(axis: Vector3, angle: number): void;
  scale_object_local(scale: Vector3): void;
  translate_object_local(offset: Vector3): void;
  rotate_x(angle: number): void;
  rotate_y(angle: number): void;
  rotate_z(angle: number): void;
  translate(offset: Vector3): void;
  orthonormalize(): void;
  set_identity(): void;
  look_at(target: Vector3, up?: Vector3, use_model_front?: boolean): void;
  look_at_from_position(position: Vector3, target: Vector3, up?: Vector3, use_model_front?: boolean): void;
  to_local(global_point: Vector3): Vector3;
  to_global(local_point: Vector3): Vector3;
}

export declare class Node3DGizmo extends RefCounted {
}

export declare class Noise extends Resource {
  get_noise_1d(x: number): number;
  get_noise_2d(x: number, y: number): number;
  get_noise_2dv(v: Vector2): number;
  get_noise_3d(x: number, y: number, z: number): number;
  get_noise_3dv(v: Vector3): number;
  get_image(width: number, height: number, invert?: boolean, in_3d_space?: boolean, normalize?: boolean): Image;
  get_seamless_image(width: number, height: number, invert?: boolean, in_3d_space?: boolean, skirt?: number, normalize?: boolean): Image;
  get_image_3d(width: number, height: number, depth: number, invert?: boolean, normalize?: boolean): any[];
  get_seamless_image_3d(width: number, height: number, depth: number, invert?: boolean, skirt?: number, normalize?: boolean): any[];
}

export declare class NoiseTexture2D extends Texture2D {
  width: number;
  height: number;
  generate_mipmaps: boolean;
  noise: Noise;
  color_ramp: Gradient;
  seamless: boolean;
  invert: boolean;
  in_3d_space: boolean;
  as_normal_map: boolean;
  normalize: boolean;
  seamless_blend_skirt: number;
  bump_strength: number;
}

export declare class NoiseTexture3D extends Texture3D {
  width: number;
  height: number;
  depth: number;
  noise: Noise;
  color_ramp: Gradient;
  seamless: boolean;
  invert: boolean;
  normalize: boolean;
  seamless_blend_skirt: number;
}

export declare class ORMMaterial3D extends BaseMaterial3D {
}

export declare class Occluder3D extends Resource {
  get_vertices(): PackedVector3Array;
  get_indices(): PackedInt32Array;
}

export declare class OccluderInstance3D extends VisualInstance3D {
  occluder: Occluder3D;
  bake_mask: number;
  bake_simplification_distance: number;
  set_bake_mask_value(layer_number: number, value: boolean): void;
  get_bake_mask_value(layer_number: number): boolean;
}

export declare class OccluderPolygon2D extends Resource {
  closed: boolean;
  cull_mode: number;
  polygon: PackedVector2Array;
}

export declare class OfflineMultiplayerPeer extends MultiplayerPeer {
}

export declare class OggPacketSequence extends Resource {
  granule_positions: PackedInt64Array;
  sampling_rate: number;
  get_length(): number;
}

export declare class OggPacketSequencePlayback extends RefCounted {
}

export declare class OmniLight3D extends Light3D {
  omni_shadow_mode: number;
}

export declare class OpenXRAPIExtension extends RefCounted {
  get_instance(): number;
  get_system_id(): number;
  get_session(): number;
  xr_result(result: number, format: string, args: any[]): boolean;
  openxr_is_enabled(check_run_in_editor: boolean): boolean;
  get_instance_proc_addr(name: string): number;
  get_error_string(result: number): string;
  get_swapchain_format_name(swapchain_format: number): string;
  set_object_name(object_type: number, object_handle: number, object_name: string): void;
  begin_debug_label_region(label_name: string): void;
  end_debug_label_region(): void;
  insert_debug_label(label_name: string): void;
  is_initialized(): boolean;
  is_running(): boolean;
  get_play_space(): number;
  get_predicted_display_time(): number;
  get_next_frame_time(): number;
  can_render(): boolean;
  find_action(name: string, action_set: RID): RID;
  action_get_handle(action: RID): number;
  get_hand_tracker(hand_index: number): number;
  register_composition_layer_provider(extension: OpenXRExtensionWrapper): void;
  unregister_composition_layer_provider(extension: OpenXRExtensionWrapper): void;
  register_projection_views_extension(extension: OpenXRExtensionWrapper): void;
  unregister_projection_views_extension(extension: OpenXRExtensionWrapper): void;
  register_frame_info_extension(extension: OpenXRExtensionWrapper): void;
  unregister_frame_info_extension(extension: OpenXRExtensionWrapper): void;
  get_render_state_z_near(): number;
  get_render_state_z_far(): number;
  set_velocity_texture(render_target: RID): void;
  set_velocity_depth_texture(render_target: RID): void;
  set_velocity_target_size(target_size: Vector2i): void;
  get_supported_swapchain_formats(): PackedInt64Array;
  openxr_swapchain_create(create_flags: number, usage_flags: number, swapchain_format: number, width: number, height: number, sample_count: number, array_size: number): number;
  openxr_swapchain_free(swapchain: number): void;
  openxr_swapchain_get_swapchain(swapchain: number): number;
  openxr_swapchain_acquire(swapchain: number): void;
  openxr_swapchain_get_image(swapchain: number): RID;
  openxr_swapchain_release(swapchain: number): void;
  get_projection_layer(): number;
  set_render_region(render_region: Rect2i): void;
  set_emulate_environment_blend_mode_alpha_blend(enabled: boolean): void;
  is_environment_blend_mode_alpha_supported(): number;
}

export declare class OpenXRAction extends Resource {
  localized_name: string;
  action_type: number;
  toplevel_paths: PackedStringArray;
}

export declare class OpenXRActionBindingModifier extends OpenXRBindingModifier {
}

export declare class OpenXRActionMap extends Resource {
  get_action_set_count(): number;
  find_action_set(name: string): OpenXRActionSet;
  get_action_set(idx: number): OpenXRActionSet;
  add_action_set(action_set: OpenXRActionSet): void;
  remove_action_set(action_set: OpenXRActionSet): void;
  get_interaction_profile_count(): number;
  find_interaction_profile(name: string): OpenXRInteractionProfile;
  get_interaction_profile(idx: number): OpenXRInteractionProfile;
  add_interaction_profile(interaction_profile: OpenXRInteractionProfile): void;
  remove_interaction_profile(interaction_profile: OpenXRInteractionProfile): void;
  create_default_action_sets(): void;
}

export declare class OpenXRActionSet extends Resource {
  localized_name: string;
  priority: number;
  get_action_count(): number;
  add_action(action: OpenXRAction): void;
  remove_action(action: OpenXRAction): void;
}

export declare class OpenXRAnalogThresholdModifier extends OpenXRActionBindingModifier {
  on_threshold: number;
  off_threshold: number;
  on_haptic: OpenXRHapticBase;
  off_haptic: OpenXRHapticBase;
}

export declare class OpenXRBindingModifier extends Resource {
}

export declare class OpenXRBindingModifierEditor extends PanelContainer {
  get_binding_modifier(): OpenXRBindingModifier;
  setup(action_map: OpenXRActionMap, binding_modifier: OpenXRBindingModifier): void;
}

export declare class OpenXRCompositionLayer extends Node3D {
  use_android_surface: boolean;
  android_surface_size: Vector2i;
  sort_order: number;
  alpha_blend: boolean;
  enable_hole_punch: boolean;
  swapchain_state_min_filter: number;
  swapchain_state_mag_filter: number;
  swapchain_state_mipmap_mode: number;
  swapchain_state_horizontal_wrap: number;
  swapchain_state_vertical_wrap: number;
  swapchain_state_red_swizzle: number;
  swapchain_state_green_swizzle: number;
  swapchain_state_blue_swizzle: number;
  swapchain_state_alpha_swizzle: number;
  swapchain_state_max_anisotropy: number;
  swapchain_state_border_color: Color;
  get_android_surface(): JavaObject;
  is_natively_supported(): boolean;
  intersects_ray(origin: Vector3, direction: Vector3): Vector2;
}

export declare class OpenXRCompositionLayerCylinder extends OpenXRCompositionLayer {
  radius: number;
  aspect_ratio: number;
  central_angle: number;
  fallback_segments: number;
}

export declare class OpenXRCompositionLayerEquirect extends OpenXRCompositionLayer {
  radius: number;
  central_horizontal_angle: number;
  upper_vertical_angle: number;
  lower_vertical_angle: number;
  fallback_segments: number;
}

export declare class OpenXRCompositionLayerQuad extends OpenXRCompositionLayer {
  quad_size: Vector2;
}

export declare class OpenXRDpadBindingModifier extends OpenXRIPBindingModifier {
  action_set: OpenXRActionSet;
  input_path: string;
  threshold: number;
  threshold_released: number;
  center_region: number;
  wedge_angle: number;
  is_sticky: boolean;
  on_haptic: OpenXRHapticBase;
  off_haptic: OpenXRHapticBase;
}

export declare class OpenXRExtensionWrapper extends Object {
  get_openxr_api(): OpenXRAPIExtension;
  register_extension_wrapper(): void;
}

export declare class OpenXRExtensionWrapperExtension extends OpenXRExtensionWrapper {
}

export declare class OpenXRFutureExtension extends OpenXRExtensionWrapper {
  is_active(): boolean;
  register_future(future: number, on_success?: Callable): OpenXRFutureResult;
  cancel_future(future: number): void;
}

export declare class OpenXRFutureResult extends RefCounted {
  get_status(): number;
  get_future(): number;
  cancel_future(): void;
  set_result_value(result_value: any): void;
  get_result_value(): any;
}

export declare class OpenXRHand extends Node3D {
  hand: number;
  motion_range: number;
  hand_skeleton: string;
  skeleton_rig: number;
  bone_update: number;
}

export declare class OpenXRHapticBase extends Resource {
}

export declare class OpenXRHapticVibration extends OpenXRHapticBase {
  duration: number;
  frequency: number;
  amplitude: number;
}

export declare class OpenXRIPBinding extends Resource {
  action: OpenXRAction;
  binding_path: string;
  paths: PackedStringArray;
  get_binding_modifier_count(): number;
  get_binding_modifier(index: number): OpenXRActionBindingModifier;
  get_path_count(): number;
  has_path(path: string): boolean;
  add_path(path: string): void;
  remove_path(path: string): void;
}

export declare class OpenXRIPBindingModifier extends OpenXRBindingModifier {
}

export declare class OpenXRInteractionProfile extends Resource {
  interaction_profile_path: string;
  get_binding_count(): number;
  get_binding(index: number): OpenXRIPBinding;
  get_binding_modifier_count(): number;
  get_binding_modifier(index: number): OpenXRIPBindingModifier;
}

export declare class OpenXRInteractionProfileEditor extends OpenXRInteractionProfileEditorBase {
}

export declare class OpenXRInteractionProfileEditorBase extends HBoxContainer {
  setup(action_map: OpenXRActionMap, interaction_profile: OpenXRInteractionProfile): void;
}

export declare class OpenXRInteractionProfileMetadata extends Object {
  register_profile_rename(old_name: string, new_name: string): void;
  register_top_level_path(display_name: string, openxr_path: string, openxr_extension_name: string): void;
  register_interaction_profile(display_name: string, openxr_path: string, openxr_extension_name: string): void;
  register_io_path(interaction_profile: string, display_name: string, toplevel_path: string, openxr_path: string, openxr_extension_name: string, action_type: number): void;
}

export declare class OpenXRInterface extends XRInterface {
  display_refresh_rate: number;
  render_target_size_multiplier: number;
  foveation_level: number;
  foveation_dynamic: boolean;
  vrs_min_radius: number;
  vrs_strength: number;
  get_session_state(): number;
  is_foveation_supported(): boolean;
  is_action_set_active(name: string): boolean;
  set_action_set_active(name: string, active: boolean): void;
  get_action_sets(): any[];
  get_available_display_refresh_rates(): any[];
  set_motion_range(hand: number, motion_range: number): void;
  get_motion_range(hand: number): number;
  get_hand_tracking_source(hand: number): number;
  get_hand_joint_flags(hand: number, joint: number): number;
  get_hand_joint_rotation(hand: number, joint: number): Quaternion;
  get_hand_joint_position(hand: number, joint: number): Vector3;
  get_hand_joint_radius(hand: number, joint: number): number;
  get_hand_joint_linear_velocity(hand: number, joint: number): Vector3;
  get_hand_joint_angular_velocity(hand: number, joint: number): Vector3;
  is_hand_tracking_supported(): boolean;
  is_hand_interaction_supported(): boolean;
  is_eye_gaze_interaction_supported(): boolean;
  set_cpu_level(level: number): void;
  set_gpu_level(level: number): void;
}

export declare class OpenXRRenderModel extends Node3D {
  render_model: RID;
  get_top_level_path(): string;
}

export declare class OpenXRRenderModelExtension extends OpenXRExtensionWrapper {
  is_active(): boolean;
  render_model_create(render_model_id: number): RID;
  render_model_destroy(render_model: RID): void;
  render_model_get_all(): any[];
  render_model_new_scene_instance(render_model: RID): Node3D;
  render_model_get_subaction_paths(render_model: RID): PackedStringArray;
  render_model_get_top_level_path(render_model: RID): string;
  render_model_get_confidence(render_model: RID): number;
  render_model_get_root_transform(render_model: RID): Transform3D;
  render_model_get_animatable_node_count(render_model: RID): number;
  render_model_get_animatable_node_name(render_model: RID, index: number): string;
  render_model_is_animatable_node_visible(render_model: RID, index: number): boolean;
  render_model_get_animatable_node_transform(render_model: RID, index: number): Transform3D;
}

export declare class OpenXRRenderModelManager extends Node3D {
  tracker: number;
  make_local_to_pose: string;
}

export declare class OpenXRVisibilityMask extends VisualInstance3D {
}

export declare class OptimizedTranslation extends Translation {
  generate(from: Translation): void;
}

export declare class OptionButton extends Button {
  fit_to_longest_item: boolean;
  allow_reselect: boolean;
  item_count: number;
  add_item(label: string, id?: number): void;
  add_icon_item(texture: Texture2D, label: string, id?: number): void;
  set_item_text(idx: number, text: string): void;
  set_item_icon(idx: number, texture: Texture2D): void;
  set_item_disabled(idx: number, disabled: boolean): void;
  set_item_id(idx: number, id: number): void;
  set_item_metadata(idx: number, metadata: any): void;
  set_item_tooltip(idx: number, tooltip: string): void;
  set_item_auto_translate_mode(idx: number, mode: number): void;
  get_item_text(idx: number): string;
  get_item_icon(idx: number): Texture2D;
  get_item_id(idx: number): number;
  get_item_index(id: number): number;
  get_item_metadata(idx: number): any;
  get_item_tooltip(idx: number): string;
  get_item_auto_translate_mode(idx: number): number;
  is_item_disabled(idx: number): boolean;
  is_item_separator(idx: number): boolean;
  add_separator(text?: string): void;
  clear(): void;
  select(idx: number): void;
  get_selected_id(): number;
  get_selected_metadata(): any;
  remove_item(idx: number): void;
  get_popup(): PopupMenu;
  show_popup(): void;
  has_selectable_items(): boolean;
  get_selectable_item(from_last?: boolean): number;
  set_disable_shortcuts(disabled: boolean): void;
}

export declare class PCKPacker extends RefCounted {
  pck_start(pck_path: string, alignment?: number, key?: string, encrypt_directory?: boolean): number;
  add_file(target_path: string, source_path: string, encrypt?: boolean): number;
  add_file_removal(target_path: string): number;
  flush(verbose?: boolean): number;
}

export declare class PackedDataContainer extends Resource {
  pack(value: any): number;
  size(): number;
}

export declare class PackedDataContainerRef extends RefCounted {
  size(): number;
}

export declare class PackedScene extends Resource {
  pack(path: Node): number;
  instantiate(edit_state?: number): Node;
  can_instantiate(): boolean;
  get_state(): SceneState;
}

export declare class PacketPeer extends RefCounted {
  encode_buffer_max_size: number;
  get_var(allow_objects?: boolean): any;
  put_var(var: any, full_objects?: boolean): number;
  get_packet(): PackedByteArray;
  put_packet(buffer: PackedByteArray): number;
  get_packet_error(): number;
  get_available_packet_count(): number;
}

export declare class PacketPeerDTLS extends PacketPeer {
  poll(): void;
  connect_to_peer(packet_peer: PacketPeerUDP, hostname: string, client_options?: TLSOptions): number;
  get_status(): number;
  disconnect_from_peer(): void;
}

export declare class PacketPeerExtension extends PacketPeer {
}

export declare class PacketPeerStream extends PacketPeer {
  input_buffer_max_size: number;
  output_buffer_max_size: number;
  stream_peer: StreamPeer;
}

export declare class PacketPeerUDP extends PacketPeer {
  bind(port: number, bind_address?: string, recv_buf_size?: number): number;
  close(): void;
  wait(): number;
  is_bound(): boolean;
  connect_to_host(host: string, port: number): number;
  is_socket_connected(): boolean;
  get_packet_ip(): string;
  get_packet_port(): number;
  get_local_port(): number;
  set_dest_address(host: string, port: number): number;
  set_broadcast_enabled(enabled: boolean): void;
  join_multicast_group(multicast_address: string, interface_name: string): number;
  leave_multicast_group(multicast_address: string, interface_name: string): number;
}

export declare class Panel extends Control {
}

export declare class PanelContainer extends Container {
}

export declare class PanoramaSkyMaterial extends Material {
  panorama: Texture2D;
  filter: boolean;
  energy_multiplier: number;
}

export declare class Parallax2D extends Node2D {
  scroll_scale: Vector2;
  scroll_offset: Vector2;
  repeat_size: Vector2;
  autoscroll: Vector2;
  repeat_times: number;
  limit_begin: Vector2;
  limit_end: Vector2;
  follow_viewport: boolean;
  ignore_camera_scroll: boolean;
  screen_offset: Vector2;
}

export declare class ParallaxBackground extends CanvasLayer {
  scroll_offset: Vector2;
  scroll_base_offset: Vector2;
  scroll_base_scale: Vector2;
  scroll_limit_begin: Vector2;
  scroll_limit_end: Vector2;
  scroll_ignore_camera_zoom: boolean;
}

export declare class ParallaxLayer extends Node2D {
  motion_scale: Vector2;
  motion_offset: Vector2;
  motion_mirroring: Vector2;
}

export declare class ParticleProcessMaterial extends Material {
  lifetime_randomness: number;
  emission_shape_offset: Vector3;
  emission_shape_scale: Vector3;
  emission_shape: number;
  emission_sphere_radius: number;
  emission_box_extents: Vector3;
  emission_point_texture: Texture2D;
  emission_normal_texture: Texture2D;
  emission_color_texture: Texture2D;
  emission_point_count: number;
  emission_ring_axis: Vector3;
  emission_ring_height: number;
  emission_ring_radius: number;
  emission_ring_inner_radius: number;
  emission_ring_cone_angle: number;
  inherit_velocity_ratio: number;
  velocity_pivot: Vector3;
  direction: Vector3;
  spread: number;
  flatness: number;
  gravity: Vector3;
  attractor_interaction_enabled: boolean;
  color: Color;
  turbulence_enabled: boolean;
  turbulence_noise_strength: number;
  turbulence_noise_scale: number;
  turbulence_noise_speed: Vector3;
  turbulence_noise_speed_random: number;
  collision_mode: number;
  collision_friction: number;
  collision_bounce: number;
  collision_use_scale: boolean;
  sub_emitter_mode: number;
  sub_emitter_frequency: number;
  sub_emitter_amount_at_end: number;
  sub_emitter_amount_at_collision: number;
  sub_emitter_amount_at_start: number;
  sub_emitter_keep_velocity: boolean;
}

export declare class Path2D extends Node2D {
  curve: Curve2D;
}

export declare class Path3D extends Node3D {
  curve: Curve3D;
  debug_custom_color: Color;
}

export declare class PathFollow2D extends Node2D {
  progress: number;
  progress_ratio: number;
  h_offset: number;
  v_offset: number;
  rotates: boolean;
  cubic_interp: boolean;
  loop: boolean;
}

export declare class PathFollow3D extends Node3D {
  progress: number;
  progress_ratio: number;
  h_offset: number;
  v_offset: number;
  rotation_mode: number;
  use_model_front: boolean;
  cubic_interp: boolean;
  loop: boolean;
  tilt_enabled: boolean;
  correct_posture(transform: Transform3D, rotation_mode: number): Transform3D;
}

export declare class PhysicalBone2D extends RigidBody2D {
  bone2d_nodepath: string;
  bone2d_index: number;
  auto_configure_joint: boolean;
  simulate_physics: boolean;
  follow_bone_when_simulating: boolean;
  get_joint(): Joint2D;
  is_simulating_physics(): boolean;
}

export declare class PhysicalBone3D extends PhysicsBody3D {
  joint_type: number;
  joint_offset: Transform3D;
  joint_rotation: Vector3;
  body_offset: Transform3D;
  mass: number;
  friction: number;
  bounce: number;
  gravity_scale: number;
  custom_integrator: boolean;
  linear_damp_mode: number;
  linear_damp: number;
  angular_damp_mode: number;
  angular_damp: number;
  linear_velocity: Vector3;
  angular_velocity: Vector3;
  can_sleep: boolean;
  apply_central_impulse(impulse: Vector3): void;
  apply_impulse(impulse: Vector3, position?: Vector3): void;
  get_simulate_physics(): boolean;
  is_simulating_physics(): boolean;
  get_bone_id(): number;
}

export declare class PhysicalBoneSimulator3D extends SkeletonModifier3D {
  is_simulating_physics(): boolean;
  physical_bones_stop_simulation(): void;
  physical_bones_start_simulation(bones?: any[]): void;
  physical_bones_add_collision_exception(exception: RID): void;
  physical_bones_remove_collision_exception(exception: RID): void;
}

export declare class PhysicalSkyMaterial extends Material {
  rayleigh_coefficient: number;
  rayleigh_color: Color;
  mie_coefficient: number;
  mie_eccentricity: number;
  mie_color: Color;
  turbidity: number;
  sun_disk_scale: number;
  ground_color: Color;
  energy_multiplier: number;
  use_debanding: boolean;
  night_sky: Texture2D;
}

export declare class PhysicsBody2D extends CollisionObject2D {
  move_and_collide(motion: Vector2, test_only?: boolean, safe_margin?: number, recovery_as_collision?: boolean): KinematicCollision2D;
  test_move(from: Transform2D, motion: Vector2, collision?: KinematicCollision2D, safe_margin?: number, recovery_as_collision?: boolean): boolean;
  get_gravity(): Vector2;
  get_collision_exceptions(): any[];
  add_collision_exception_with(body: Node): void;
  remove_collision_exception_with(body: Node): void;
}

export declare class PhysicsBody3D extends CollisionObject3D {
  move_and_collide(motion: Vector3, test_only?: boolean, safe_margin?: number, recovery_as_collision?: boolean, max_collisions?: number): KinematicCollision3D;
  test_move(from: Transform3D, motion: Vector3, collision?: KinematicCollision3D, safe_margin?: number, recovery_as_collision?: boolean, max_collisions?: number): boolean;
  get_gravity(): Vector3;
  get_collision_exceptions(): any[];
  add_collision_exception_with(body: Node): void;
  remove_collision_exception_with(body: Node): void;
}

export declare class PhysicsDirectBodyState2D extends Object {
  readonly step: number;
  readonly inverse_mass: number;
  readonly inverse_inertia: number;
  readonly total_angular_damp: number;
  readonly total_linear_damp: number;
  readonly total_gravity: Vector2;
  readonly center_of_mass: Vector2;
  readonly center_of_mass_local: Vector2;
  angular_velocity: number;
  linear_velocity: Vector2;
  sleeping: boolean;
  collision_layer: number;
  collision_mask: number;
  transform: Transform2D;
  get_velocity_at_local_position(local_position: Vector2): Vector2;
  apply_central_impulse(impulse: Vector2): void;
  apply_torque_impulse(impulse: number): void;
  apply_impulse(impulse: Vector2, position?: Vector2): void;
  apply_central_force(force?: Vector2): void;
  apply_force(force: Vector2, position?: Vector2): void;
  apply_torque(torque: number): void;
  add_constant_central_force(force?: Vector2): void;
  add_constant_force(force: Vector2, position?: Vector2): void;
  add_constant_torque(torque: number): void;
  set_constant_force(force: Vector2): void;
  get_constant_force(): Vector2;
  set_constant_torque(torque: number): void;
  get_constant_torque(): number;
  get_contact_count(): number;
  get_contact_local_position(contact_idx: number): Vector2;
  get_contact_local_normal(contact_idx: number): Vector2;
  get_contact_local_shape(contact_idx: number): number;
  get_contact_local_velocity_at_position(contact_idx: number): Vector2;
  get_contact_collider(contact_idx: number): RID;
  get_contact_collider_position(contact_idx: number): Vector2;
  get_contact_collider_id(contact_idx: number): number;
  get_contact_collider_object(contact_idx: number): any;
  get_contact_collider_shape(contact_idx: number): number;
  get_contact_collider_velocity_at_position(contact_idx: number): Vector2;
  get_contact_impulse(contact_idx: number): Vector2;
  integrate_forces(): void;
  get_space_state(): PhysicsDirectSpaceState2D;
}

export declare class PhysicsDirectBodyState2DExtension extends PhysicsDirectBodyState2D {
}

export declare class PhysicsDirectBodyState3D extends Object {
  readonly step: number;
  readonly inverse_mass: number;
  readonly total_angular_damp: number;
  readonly total_linear_damp: number;
  readonly inverse_inertia: Vector3;
  readonly inverse_inertia_tensor: Basis;
  readonly total_gravity: Vector3;
  readonly center_of_mass: Vector3;
  readonly center_of_mass_local: Vector3;
  readonly principal_inertia_axes: Basis;
  angular_velocity: Vector3;
  linear_velocity: Vector3;
  sleeping: boolean;
  collision_layer: number;
  collision_mask: number;
  transform: Transform3D;
  get_velocity_at_local_position(local_position: Vector3): Vector3;
  apply_central_impulse(impulse?: Vector3): void;
  apply_impulse(impulse: Vector3, position?: Vector3): void;
  apply_torque_impulse(impulse: Vector3): void;
  apply_central_force(force?: Vector3): void;
  apply_force(force: Vector3, position?: Vector3): void;
  apply_torque(torque: Vector3): void;
  add_constant_central_force(force?: Vector3): void;
  add_constant_force(force: Vector3, position?: Vector3): void;
  add_constant_torque(torque: Vector3): void;
  set_constant_force(force: Vector3): void;
  get_constant_force(): Vector3;
  set_constant_torque(torque: Vector3): void;
  get_constant_torque(): Vector3;
  get_contact_count(): number;
  get_contact_local_position(contact_idx: number): Vector3;
  get_contact_local_normal(contact_idx: number): Vector3;
  get_contact_impulse(contact_idx: number): Vector3;
  get_contact_local_shape(contact_idx: number): number;
  get_contact_local_velocity_at_position(contact_idx: number): Vector3;
  get_contact_collider(contact_idx: number): RID;
  get_contact_collider_position(contact_idx: number): Vector3;
  get_contact_collider_id(contact_idx: number): number;
  get_contact_collider_object(contact_idx: number): any;
  get_contact_collider_shape(contact_idx: number): number;
  get_contact_collider_velocity_at_position(contact_idx: number): Vector3;
  integrate_forces(): void;
  get_space_state(): PhysicsDirectSpaceState3D;
}

export declare class PhysicsDirectBodyState3DExtension extends PhysicsDirectBodyState3D {
}

export declare class PhysicsDirectSpaceState2D extends Object {
  intersect_point(parameters: PhysicsPointQueryParameters2D, max_results?: number): any[];
  intersect_ray(parameters: PhysicsRayQueryParameters2D): Record<string, any>;
  intersect_shape(parameters: PhysicsShapeQueryParameters2D, max_results?: number): any[];
  cast_motion(parameters: PhysicsShapeQueryParameters2D): PackedFloat32Array;
  collide_shape(parameters: PhysicsShapeQueryParameters2D, max_results?: number): any[];
  get_rest_info(parameters: PhysicsShapeQueryParameters2D): Record<string, any>;
}

export declare class PhysicsDirectSpaceState2DExtension extends PhysicsDirectSpaceState2D {
  is_body_excluded_from_query(body: RID): boolean;
}

export declare class PhysicsDirectSpaceState3D extends Object {
  intersect_point(parameters: PhysicsPointQueryParameters3D, max_results?: number): any[];
  intersect_ray(parameters: PhysicsRayQueryParameters3D): Record<string, any>;
  intersect_shape(parameters: PhysicsShapeQueryParameters3D, max_results?: number): any[];
  cast_motion(parameters: PhysicsShapeQueryParameters3D): PackedFloat32Array;
  collide_shape(parameters: PhysicsShapeQueryParameters3D, max_results?: number): any[];
  get_rest_info(parameters: PhysicsShapeQueryParameters3D): Record<string, any>;
}

export declare class PhysicsDirectSpaceState3DExtension extends PhysicsDirectSpaceState3D {
  is_body_excluded_from_query(body: RID): boolean;
}

export declare class PhysicsMaterial extends Resource {
  friction: number;
  rough: boolean;
  bounce: number;
  absorbent: boolean;
}

export declare class PhysicsPointQueryParameters2D extends RefCounted {
  position: Vector2;
  canvas_instance_id: number;
  collision_mask: number;
  exclude: any[];
  collide_with_bodies: boolean;
  collide_with_areas: boolean;
}

export declare class PhysicsPointQueryParameters3D extends RefCounted {
  position: Vector3;
  collision_mask: number;
  exclude: any[];
  collide_with_bodies: boolean;
  collide_with_areas: boolean;
}

export declare class PhysicsRayQueryParameters2D extends RefCounted {
  from: Vector2;
  to: Vector2;
  collision_mask: number;
  exclude: any[];
  collide_with_bodies: boolean;
  collide_with_areas: boolean;
  hit_from_inside: boolean;
  create(from: Vector2, to: Vector2, collision_mask?: number, exclude?: any[]): PhysicsRayQueryParameters2D;
}

export declare class PhysicsRayQueryParameters3D extends RefCounted {
  from: Vector3;
  to: Vector3;
  collision_mask: number;
  exclude: any[];
  collide_with_bodies: boolean;
  collide_with_areas: boolean;
  hit_from_inside: boolean;
  hit_back_faces: boolean;
  create(from: Vector3, to: Vector3, collision_mask?: number, exclude?: any[]): PhysicsRayQueryParameters3D;
}

export declare class PhysicsServer2DExtension extends PhysicsServer2D {
  body_test_motion_is_excluding_body(body: RID): boolean;
  body_test_motion_is_excluding_object(object: number): boolean;
}

export declare class PhysicsServer3DExtension extends PhysicsServer3D {
  body_test_motion_is_excluding_body(body: RID): boolean;
  body_test_motion_is_excluding_object(object: number): boolean;
}

export declare class PhysicsShapeQueryParameters2D extends RefCounted {
  collision_mask: number;
  exclude: any[];
  margin: number;
  motion: Vector2;
  shape_rid: RID;
  transform: Transform2D;
  collide_with_bodies: boolean;
  collide_with_areas: boolean;
}

export declare class PhysicsShapeQueryParameters3D extends RefCounted {
  collision_mask: number;
  exclude: any[];
  margin: number;
  motion: Vector3;
  shape_rid: RID;
  transform: Transform3D;
  collide_with_bodies: boolean;
  collide_with_areas: boolean;
}

export declare class PhysicsTestMotionParameters2D extends RefCounted {
  from: Transform2D;
  motion: Vector2;
  margin: number;
  collide_separation_ray: boolean;
  exclude_bodies: any[];
  recovery_as_collision: boolean;
}

export declare class PhysicsTestMotionParameters3D extends RefCounted {
  from: Transform3D;
  motion: Vector3;
  margin: number;
  max_collisions: number;
  collide_separation_ray: boolean;
  exclude_bodies: any[];
  recovery_as_collision: boolean;
}

export declare class PhysicsTestMotionResult2D extends RefCounted {
  get_travel(): Vector2;
  get_remainder(): Vector2;
  get_collision_point(): Vector2;
  get_collision_normal(): Vector2;
  get_collider_velocity(): Vector2;
  get_collider_id(): number;
  get_collider_rid(): RID;
  get_collider(): any;
  get_collider_shape(): number;
  get_collision_local_shape(): number;
  get_collision_depth(): number;
  get_collision_safe_fraction(): number;
  get_collision_unsafe_fraction(): number;
}

export declare class PhysicsTestMotionResult3D extends RefCounted {
  get_travel(): Vector3;
  get_remainder(): Vector3;
  get_collision_safe_fraction(): number;
  get_collision_unsafe_fraction(): number;
  get_collision_count(): number;
  get_collision_point(collision_index?: number): Vector3;
  get_collision_normal(collision_index?: number): Vector3;
  get_collider_velocity(collision_index?: number): Vector3;
  get_collider_id(collision_index?: number): number;
  get_collider_rid(collision_index?: number): RID;
  get_collider(collision_index?: number): any;
  get_collider_shape(collision_index?: number): number;
  get_collision_local_shape(collision_index?: number): number;
  get_collision_depth(collision_index?: number): number;
}

export declare class PinJoint2D extends Joint2D {
  softness: number;
  angular_limit_enabled: boolean;
  angular_limit_lower: number;
  angular_limit_upper: number;
  motor_enabled: boolean;
  motor_target_velocity: number;
}

export declare class PinJoint3D extends Joint3D {
  set_param(param: number, value: number): void;
  get_param(param: number): number;
}

export declare class PlaceholderCubemap extends PlaceholderTextureLayered {
}

export declare class PlaceholderCubemapArray extends PlaceholderTextureLayered {
}

export declare class PlaceholderMaterial extends Material {
}

export declare class PlaceholderMesh extends Mesh {
  aabb: AABB;
}

export declare class PlaceholderTexture2D extends Texture2D {
  size: Vector2;
}

export declare class PlaceholderTexture2DArray extends PlaceholderTextureLayered {
}

export declare class PlaceholderTexture3D extends Texture3D {
  size: Vector3i;
}

export declare class PlaceholderTextureLayered extends TextureLayered {
  size: Vector2i;
  layers: number;
}

export declare class PlaneMesh extends PrimitiveMesh {
  size: Vector2;
  subdivide_width: number;
  subdivide_depth: number;
  center_offset: Vector3;
  orientation: number;
}

export declare class PointLight2D extends Light2D {
  offset: Vector2;
  texture_scale: number;
  height: number;
}

export declare class PointMesh extends PrimitiveMesh {
}

export declare class Polygon2D extends Node2D {
  color: Color;
  offset: Vector2;
  antialiased: boolean;
  texture: Texture2D;
  texture_offset: Vector2;
  texture_scale: Vector2;
  texture_rotation: number;
  skeleton: string;
  invert_enabled: boolean;
  invert_border: number;
  polygon: PackedVector2Array;
  uv: PackedVector2Array;
  vertex_colors: PackedColorArray;
  polygons: any[];
  internal_vertex_count: number;
  add_bone(path: string, weights: PackedFloat32Array): void;
  get_bone_count(): number;
  get_bone_path(index: number): string;
  get_bone_weights(index: number): PackedFloat32Array;
  erase_bone(index: number): void;
  clear_bones(): void;
  set_bone_path(index: number, path: string): void;
  set_bone_weights(index: number, weights: PackedFloat32Array): void;
}

export declare class PolygonOccluder3D extends Occluder3D {
  polygon: PackedVector2Array;
}

export declare class PolygonPathFinder extends Resource {
  setup(points: PackedVector2Array, connections: PackedInt32Array): void;
  find_path(from: Vector2, to: Vector2): PackedVector2Array;
  get_intersections(from: Vector2, to: Vector2): PackedVector2Array;
  get_closest_point(point: Vector2): Vector2;
  is_point_inside(point: Vector2): boolean;
  set_point_penalty(idx: number, penalty: number): void;
  get_point_penalty(idx: number): number;
  get_bounds(): Rect2;
}

export declare class Popup extends Window {
}

export declare class PopupMenu extends Popup {
  hide_on_item_selection: boolean;
  hide_on_checkable_item_selection: boolean;
  hide_on_state_item_selection: boolean;
  submenu_popup_delay: number;
  allow_search: boolean;
  system_menu_id: number;
  prefer_native_menu: boolean;
  item_count: number;
  activate_item_by_event(event: InputEvent, for_global_only?: boolean): boolean;
  is_native_menu(): boolean;
  add_item(label: string, id?: number, accel?: number): void;
  add_icon_item(texture: Texture2D, label: string, id?: number, accel?: number): void;
  add_check_item(label: string, id?: number, accel?: number): void;
  add_icon_check_item(texture: Texture2D, label: string, id?: number, accel?: number): void;
  add_radio_check_item(label: string, id?: number, accel?: number): void;
  add_icon_radio_check_item(texture: Texture2D, label: string, id?: number, accel?: number): void;
  add_multistate_item(label: string, max_states: number, default_state?: number, id?: number, accel?: number): void;
  add_shortcut(shortcut: Shortcut, id?: number, global?: boolean, allow_echo?: boolean): void;
  add_icon_shortcut(texture: Texture2D, shortcut: Shortcut, id?: number, global?: boolean, allow_echo?: boolean): void;
  add_check_shortcut(shortcut: Shortcut, id?: number, global?: boolean): void;
  add_icon_check_shortcut(texture: Texture2D, shortcut: Shortcut, id?: number, global?: boolean): void;
  add_radio_check_shortcut(shortcut: Shortcut, id?: number, global?: boolean): void;
  add_icon_radio_check_shortcut(texture: Texture2D, shortcut: Shortcut, id?: number, global?: boolean): void;
  add_submenu_item(label: string, submenu: string, id?: number): void;
  add_submenu_node_item(label: string, submenu: PopupMenu, id?: number): void;
  set_item_text(index: number, text: string): void;
  set_item_text_direction(index: number, direction: number): void;
  set_item_language(index: number, language: string): void;
  set_item_auto_translate_mode(index: number, mode: number): void;
  set_item_icon(index: number, icon: Texture2D): void;
  set_item_icon_max_width(index: number, width: number): void;
  set_item_icon_modulate(index: number, modulate: Color): void;
  set_item_checked(index: number, checked: boolean): void;
  set_item_id(index: number, id: number): void;
  set_item_accelerator(index: number, accel: number): void;
  set_item_metadata(index: number, metadata: any): void;
  set_item_disabled(index: number, disabled: boolean): void;
  set_item_submenu(index: number, submenu: string): void;
  set_item_submenu_node(index: number, submenu: PopupMenu): void;
  set_item_as_separator(index: number, enable: boolean): void;
  set_item_as_checkable(index: number, enable: boolean): void;
  set_item_as_radio_checkable(index: number, enable: boolean): void;
  set_item_tooltip(index: number, tooltip: string): void;
  set_item_shortcut(index: number, shortcut: Shortcut, global?: boolean): void;
  set_item_indent(index: number, indent: number): void;
  set_item_multistate(index: number, state: number): void;
  set_item_multistate_max(index: number, max_states: number): void;
  set_item_shortcut_disabled(index: number, disabled: boolean): void;
  toggle_item_checked(index: number): void;
  toggle_item_multistate(index: number): void;
  get_item_text(index: number): string;
  get_item_text_direction(index: number): number;
  get_item_language(index: number): string;
  get_item_auto_translate_mode(index: number): number;
  get_item_icon(index: number): Texture2D;
  get_item_icon_max_width(index: number): number;
  get_item_icon_modulate(index: number): Color;
  is_item_checked(index: number): boolean;
  get_item_id(index: number): number;
  get_item_index(id: number): number;
  get_item_accelerator(index: number): number;
  get_item_metadata(index: number): any;
  is_item_disabled(index: number): boolean;
  get_item_submenu(index: number): string;
  get_item_submenu_node(index: number): PopupMenu;
  is_item_separator(index: number): boolean;
  is_item_checkable(index: number): boolean;
  is_item_radio_checkable(index: number): boolean;
  is_item_shortcut_disabled(index: number): boolean;
  get_item_tooltip(index: number): string;
  get_item_shortcut(index: number): Shortcut;
  get_item_indent(index: number): number;
  get_item_multistate_max(index: number): number;
  get_item_multistate(index: number): number;
  set_focused_item(index: number): void;
  get_focused_item(): number;
  scroll_to_item(index: number): void;
  remove_item(index: number): void;
  add_separator(label?: string, id?: number): void;
  clear(free_submenus?: boolean): void;
  is_system_menu(): boolean;
}

export declare class PopupPanel extends Popup {
}

export declare class PortableCompressedTexture2D extends Texture2D {
  size_override: Vector2;
  keep_compressed_buffer: boolean;
  create_from_image(image: Image, compression_mode: number, normal_map?: boolean, lossy_quality?: number): void;
  get_format(): number;
  get_compression_mode(): number;
  set_basisu_compressor_params(uastc_level: number, rdo_quality_loss: number): void;
  set_keep_all_compressed_buffers(keep: boolean): void;
  is_keeping_all_compressed_buffers(): boolean;
}

export declare class PrimitiveMesh extends Mesh {
  custom_aabb: AABB;
  flip_faces: boolean;
  add_uv2: boolean;
  uv2_padding: number;
  get_mesh_arrays(): any[];
  request_update(): void;
}

export declare class PrismMesh extends PrimitiveMesh {
  left_to_right: number;
  size: Vector3;
  subdivide_width: number;
  subdivide_height: number;
  subdivide_depth: number;
}

export declare class ProceduralSkyMaterial extends Material {
  sky_top_color: Color;
  sky_horizon_color: Color;
  sky_curve: number;
  sky_energy_multiplier: number;
  sky_cover: Texture2D;
  sky_cover_modulate: Color;
  ground_bottom_color: Color;
  ground_horizon_color: Color;
  ground_curve: number;
  ground_energy_multiplier: number;
  sun_angle_max: number;
  sun_curve: number;
  use_debanding: boolean;
  energy_multiplier: number;
}

export declare class ProgressBar extends Range {
  fill_mode: number;
  show_percentage: boolean;
  indeterminate: boolean;
  editor_preview_indeterminate: boolean;
}

export declare class PropertyTweener extends Tweener {
  from(value: any): PropertyTweener;
  from_current(): PropertyTweener;
  as_relative(): PropertyTweener;
  set_trans(trans: number): PropertyTweener;
  set_ease(ease: number): PropertyTweener;
  set_custom_interpolator(interpolator_method: Callable): PropertyTweener;
  set_delay(delay: number): PropertyTweener;
}

export declare class QuadMesh extends PlaneMesh {
}

export declare class QuadOccluder3D extends Occluder3D {
  size: Vector2;
}

export declare class RDAttachmentFormat extends RefCounted {
  format: number;
  samples: number;
  usage_flags: number;
}

export declare class RDFramebufferPass extends RefCounted {
  color_attachments: PackedInt32Array;
  input_attachments: PackedInt32Array;
  resolve_attachments: PackedInt32Array;
  preserve_attachments: PackedInt32Array;
  depth_attachment: number;
}

export declare class RDPipelineColorBlendState extends RefCounted {
  enable_logic_op: boolean;
  logic_op: number;
  blend_constant: Color;
  attachments: any[];
}

export declare class RDPipelineColorBlendStateAttachment extends RefCounted {
  enable_blend: boolean;
  src_color_blend_factor: number;
  dst_color_blend_factor: number;
  color_blend_op: number;
  src_alpha_blend_factor: number;
  dst_alpha_blend_factor: number;
  alpha_blend_op: number;
  write_r: boolean;
  write_g: boolean;
  write_b: boolean;
  write_a: boolean;
  set_as_mix(): void;
}

export declare class RDPipelineDepthStencilState extends RefCounted {
  enable_depth_test: boolean;
  enable_depth_write: boolean;
  depth_compare_operator: number;
  enable_depth_range: boolean;
  depth_range_min: number;
  depth_range_max: number;
  enable_stencil: boolean;
  front_op_fail: number;
  front_op_pass: number;
  front_op_depth_fail: number;
  front_op_compare: number;
  front_op_compare_mask: number;
  front_op_write_mask: number;
  front_op_reference: number;
  back_op_fail: number;
  back_op_pass: number;
  back_op_depth_fail: number;
  back_op_compare: number;
  back_op_compare_mask: number;
  back_op_write_mask: number;
  back_op_reference: number;
}

export declare class RDPipelineMultisampleState extends RefCounted {
  sample_count: number;
  enable_sample_shading: boolean;
  min_sample_shading: number;
  enable_alpha_to_coverage: boolean;
  enable_alpha_to_one: boolean;
  sample_masks: any[];
}

export declare class RDPipelineRasterizationState extends RefCounted {
  enable_depth_clamp: boolean;
  discard_primitives: boolean;
  wireframe: boolean;
  cull_mode: number;
  front_face: number;
  depth_bias_enabled: boolean;
  depth_bias_constant_factor: number;
  depth_bias_clamp: number;
  depth_bias_slope_factor: number;
  line_width: number;
  patch_control_points: number;
}

export declare class RDPipelineSpecializationConstant extends RefCounted {
  value: any;
  constant_id: number;
}

export declare class RDSamplerState extends RefCounted {
  mag_filter: number;
  min_filter: number;
  mip_filter: number;
  repeat_u: number;
  repeat_v: number;
  repeat_w: number;
  lod_bias: number;
  use_anisotropy: boolean;
  anisotropy_max: number;
  enable_compare: boolean;
  compare_op: number;
  min_lod: number;
  max_lod: number;
  border_color: number;
  unnormalized_uvw: boolean;
}

export declare class RDShaderFile extends Resource {
  base_error: string;
  set_bytecode(bytecode: RDShaderSPIRV, version?: string): void;
  get_spirv(version?: string): RDShaderSPIRV;
  get_version_list(): any[];
}

export declare class RDShaderSPIRV extends Resource {
}

export declare class RDShaderSource extends RefCounted {
  language: number;
}

export declare class RDTextureFormat extends RefCounted {
  format: number;
  width: number;
  height: number;
  depth: number;
  array_layers: number;
  mipmaps: number;
  texture_type: number;
  samples: number;
  usage_bits: number;
  is_resolve_buffer: boolean;
  is_discardable: boolean;
  add_shareable_format(format: number): void;
  remove_shareable_format(format: number): void;
}

export declare class RDTextureView extends RefCounted {
  format_override: number;
  swizzle_r: number;
  swizzle_g: number;
  swizzle_b: number;
  swizzle_a: number;
}

export declare class RDUniform extends RefCounted {
  uniform_type: number;
  binding: number;
  add_id(id: RID): void;
  clear_ids(): void;
  get_ids(): any[];
}

export declare class RDVertexAttribute extends RefCounted {
  location: number;
  offset: number;
  format: number;
  stride: number;
  frequency: number;
}

export declare class RandomNumberGenerator extends RefCounted {
  seed: number;
  state: number;
  randi(): number;
  randf(): number;
  randfn(mean?: number, deviation?: number): number;
  randf_range(from: number, to: number): number;
  randi_range(from: number, to: number): number;
  rand_weighted(weights: PackedFloat32Array): number;
  randomize(): void;
}

export declare class Range extends Control {
  min_value: number;
  max_value: number;
  step: number;
  page: number;
  value: number;
  ratio: number;
  exp_edit: boolean;
  rounded: boolean;
  allow_greater: boolean;
  allow_lesser: boolean;
  set_value_no_signal(value: number): void;
  share(with: Node): void;
  unshare(): void;
}

export declare class RayCast2D extends Node2D {
  enabled: boolean;
  exclude_parent: boolean;
  target_position: Vector2;
  collision_mask: number;
  hit_from_inside: boolean;
  collide_with_areas: boolean;
  collide_with_bodies: boolean;
  is_colliding(): boolean;
  force_raycast_update(): void;
  get_collider(): any;
  get_collider_rid(): RID;
  get_collider_shape(): number;
  get_collision_point(): Vector2;
  get_collision_normal(): Vector2;
  add_exception_rid(rid: RID): void;
  add_exception(node: CollisionObject2D): void;
  remove_exception_rid(rid: RID): void;
  remove_exception(node: CollisionObject2D): void;
  clear_exceptions(): void;
  set_collision_mask_value(layer_number: number, value: boolean): void;
  get_collision_mask_value(layer_number: number): boolean;
}

export declare class RayCast3D extends Node3D {
  enabled: boolean;
  exclude_parent: boolean;
  target_position: Vector3;
  collision_mask: number;
  hit_from_inside: boolean;
  hit_back_faces: boolean;
  collide_with_areas: boolean;
  collide_with_bodies: boolean;
  debug_shape_custom_color: Color;
  debug_shape_thickness: number;
  is_colliding(): boolean;
  force_raycast_update(): void;
  get_collider(): any;
  get_collider_rid(): RID;
  get_collider_shape(): number;
  get_collision_point(): Vector3;
  get_collision_normal(): Vector3;
  get_collision_face_index(): number;
  add_exception_rid(rid: RID): void;
  add_exception(node: CollisionObject3D): void;
  remove_exception_rid(rid: RID): void;
  remove_exception(node: CollisionObject3D): void;
  clear_exceptions(): void;
  set_collision_mask_value(layer_number: number, value: boolean): void;
  get_collision_mask_value(layer_number: number): boolean;
}

export declare class RectangleShape2D extends Shape2D {
  size: Vector2;
}

export declare class RefCounted extends Object {
  init_ref(): boolean;
  get_reference_count(): number;
}

export declare class ReferenceRect extends Control {
  border_color: Color;
  border_width: number;
  editor_only: boolean;
}

export declare class ReflectionProbe extends VisualInstance3D {
  update_mode: number;
  intensity: number;
  blend_distance: number;
  max_distance: number;
  size: Vector3;
  origin_offset: Vector3;
  box_projection: boolean;
  interior: boolean;
  enable_shadows: boolean;
  cull_mask: number;
  reflection_mask: number;
  mesh_lod_threshold: number;
  ambient_mode: number;
  ambient_color: Color;
  ambient_color_energy: number;
}

export declare class RegEx extends RefCounted {
  create_from_string(pattern: string, show_error?: boolean): RegEx;
  clear(): void;
  compile(pattern: string, show_error?: boolean): number;
  search(subject: string, offset?: number, end?: number): RegExMatch;
  search_all(subject: string, offset?: number, end?: number): any[];
  sub(subject: string, replacement: string, all?: boolean, offset?: number, end?: number): string;
  is_valid(): boolean;
  get_pattern(): string;
  get_group_count(): number;
  get_names(): PackedStringArray;
}

export declare class RegExMatch extends RefCounted {
  readonly subject: string;
  readonly names: Record<string, any>;
  get_group_count(): number;
  get_string(name?: any): string;
  get_start(name?: any): number;
  get_end(name?: any): number;
}

export declare class RemoteTransform2D extends Node2D {
  remote_path: string;
  use_global_coordinates: boolean;
  update_position: boolean;
  update_rotation: boolean;
  update_scale: boolean;
  force_update_cache(): void;
}

export declare class RemoteTransform3D extends Node3D {
  remote_path: string;
  use_global_coordinates: boolean;
  update_position: boolean;
  update_rotation: boolean;
  update_scale: boolean;
  force_update_cache(): void;
}

export declare class RenderData extends Object {
  get_render_scene_buffers(): RenderSceneBuffers;
  get_render_scene_data(): RenderSceneData;
  get_environment(): RID;
  get_camera_attributes(): RID;
}

export declare class RenderDataExtension extends RenderData {
}

export declare class RenderDataRD extends RenderData {
}

export declare class RenderSceneBuffers extends RefCounted {
  configure(config: RenderSceneBuffersConfiguration): void;
}

export declare class RenderSceneBuffersConfiguration extends RefCounted {
  render_target: RID;
  internal_size: Vector2i;
  target_size: Vector2i;
  view_count: number;
  scaling_3d_mode: number;
  msaa_3d: number;
  screen_space_aa: number;
  anisotropic_filtering_level: number;
}

export declare class RenderSceneBuffersExtension extends RenderSceneBuffers {
}

export declare class RenderSceneBuffersRD extends RenderSceneBuffers {
  has_texture(context: string, name: string): boolean;
  create_texture(context: string, name: string, data_format: number, usage_bits: number, texture_samples: number, size: Vector2i, layers: number, mipmaps: number, unique: boolean, discardable: boolean): RID;
  create_texture_from_format(context: string, name: string, format: RDTextureFormat, view: RDTextureView, unique: boolean): RID;
  create_texture_view(context: string, name: string, view_name: string, view: RDTextureView): RID;
  get_texture(context: string, name: string): RID;
  get_texture_format(context: string, name: string): RDTextureFormat;
  get_texture_slice(context: string, name: string, layer: number, mipmap: number, layers: number, mipmaps: number): RID;
  get_texture_slice_view(context: string, name: string, layer: number, mipmap: number, layers: number, mipmaps: number, view: RDTextureView): RID;
  get_texture_slice_size(context: string, name: string, mipmap: number): Vector2i;
  clear_context(context: string): void;
  get_color_texture(msaa?: boolean): RID;
  get_color_layer(layer: number, msaa?: boolean): RID;
  get_depth_texture(msaa?: boolean): RID;
  get_depth_layer(layer: number, msaa?: boolean): RID;
  get_velocity_texture(msaa?: boolean): RID;
  get_velocity_layer(layer: number, msaa?: boolean): RID;
  get_render_target(): RID;
  get_view_count(): number;
  get_internal_size(): Vector2i;
  get_target_size(): Vector2i;
  get_scaling_3d_mode(): number;
  get_fsr_sharpness(): number;
  get_msaa_3d(): number;
  get_texture_samples(): number;
  get_screen_space_aa(): number;
  get_use_taa(): boolean;
  get_use_debanding(): boolean;
}

export declare class RenderSceneData extends Object {
  get_cam_transform(): Transform3D;
  get_cam_projection(): Projection;
  get_view_count(): number;
  get_view_eye_offset(view: number): Vector3;
  get_view_projection(view: number): Projection;
  get_uniform_buffer(): RID;
}

export declare class RenderSceneDataExtension extends RenderSceneData {
}

export declare class RenderSceneDataRD extends RenderSceneData {
}

export declare class RenderingDevice extends Object {
  texture_create(format: RDTextureFormat, view: RDTextureView, data?: any[]): RID;
  texture_create_shared(view: RDTextureView, with_texture: RID): RID;
  texture_create_shared_from_slice(view: RDTextureView, with_texture: RID, layer: number, mipmap: number, mipmaps?: number, slice_type?: number): RID;
  texture_create_from_extension(type: number, format: number, samples: number, usage_flags: number, image: number, width: number, height: number, depth: number, layers: number, mipmaps?: number): RID;
  texture_update(texture: RID, layer: number, data: PackedByteArray): number;
  texture_get_data(texture: RID, layer: number): PackedByteArray;
  texture_get_data_async(texture: RID, layer: number, callback: Callable): number;
  texture_is_format_supported_for_usage(format: number, usage_flags: number): boolean;
  texture_is_shared(texture: RID): boolean;
  texture_is_valid(texture: RID): boolean;
  texture_set_discardable(texture: RID, discardable: boolean): void;
  texture_is_discardable(texture: RID): boolean;
  texture_copy(from_texture: RID, to_texture: RID, from_pos: Vector3, to_pos: Vector3, size: Vector3, src_mipmap: number, dst_mipmap: number, src_layer: number, dst_layer: number): number;
  texture_clear(texture: RID, color: Color, base_mipmap: number, mipmap_count: number, base_layer: number, layer_count: number): number;
  texture_resolve_multisample(from_texture: RID, to_texture: RID): number;
  texture_get_format(texture: RID): RDTextureFormat;
  texture_get_native_handle(texture: RID): number;
  framebuffer_format_create(attachments: any[], view_count?: number): number;
  framebuffer_format_create_multipass(attachments: any[], passes: any[], view_count?: number): number;
  framebuffer_format_create_empty(samples?: number): number;
  framebuffer_format_get_texture_samples(format: number, render_pass?: number): number;
  framebuffer_create(textures: any[], validate_with_format?: number, view_count?: number): RID;
  framebuffer_create_multipass(textures: any[], passes: any[], validate_with_format?: number, view_count?: number): RID;
  framebuffer_create_empty(size: Vector2i, samples?: number, validate_with_format?: number): RID;
  framebuffer_get_format(framebuffer: RID): number;
  framebuffer_is_valid(framebuffer: RID): boolean;
  sampler_create(state: RDSamplerState): RID;
  sampler_is_format_supported_for_filter(format: number, sampler_filter: number): boolean;
  vertex_buffer_create(size_bytes: number, data?: PackedByteArray, creation_bits?: number): RID;
  vertex_format_create(vertex_descriptions: any[]): number;
  vertex_array_create(vertex_count: number, vertex_format: number, src_buffers: any[], offsets?: PackedInt64Array): RID;
  index_buffer_create(size_indices: number, format: number, data?: PackedByteArray, use_restart_indices?: boolean, creation_bits?: number): RID;
  index_array_create(index_buffer: RID, index_offset: number, index_count: number): RID;
  shader_compile_spirv_from_source(shader_source: RDShaderSource, allow_cache?: boolean): RDShaderSPIRV;
  shader_compile_binary_from_spirv(spirv_data: RDShaderSPIRV, name?: string): PackedByteArray;
  shader_create_from_spirv(spirv_data: RDShaderSPIRV, name?: string): RID;
  shader_create_from_bytecode(binary_data: PackedByteArray, placeholder_rid?: RID): RID;
  shader_create_placeholder(): RID;
  shader_get_vertex_input_attribute_mask(shader: RID): number;
  uniform_buffer_create(size_bytes: number, data?: PackedByteArray, creation_bits?: number): RID;
  storage_buffer_create(size_bytes: number, data?: PackedByteArray, usage?: number, creation_bits?: number): RID;
  texture_buffer_create(size_bytes: number, format: number, data?: PackedByteArray): RID;
  uniform_set_create(uniforms: any[], shader: RID, shader_set: number): RID;
  uniform_set_is_valid(uniform_set: RID): boolean;
  buffer_copy(src_buffer: RID, dst_buffer: RID, src_offset: number, dst_offset: number, size: number): number;
  buffer_update(buffer: RID, offset: number, size_bytes: number, data: PackedByteArray): number;
  buffer_clear(buffer: RID, offset: number, size_bytes: number): number;
  buffer_get_data(buffer: RID, offset_bytes?: number, size_bytes?: number): PackedByteArray;
  buffer_get_data_async(buffer: RID, callback: Callable, offset_bytes?: number, size_bytes?: number): number;
  buffer_get_device_address(buffer: RID): number;
  render_pipeline_create(shader: RID, framebuffer_format: number, vertex_format: number, primitive: number, rasterization_state: RDPipelineRasterizationState, multisample_state: RDPipelineMultisampleState, stencil_state: RDPipelineDepthStencilState, color_blend_state: RDPipelineColorBlendState, dynamic_state_flags?: number, for_render_pass?: number, specialization_constants?: any[]): RID;
  render_pipeline_is_valid(render_pipeline: RID): boolean;
  compute_pipeline_create(shader: RID, specialization_constants?: any[]): RID;
  compute_pipeline_is_valid(compute_pipeline: RID): boolean;
  screen_get_width(screen?: number): number;
  screen_get_height(screen?: number): number;
  screen_get_framebuffer_format(screen?: number): number;
  draw_list_begin_for_screen(screen?: number, clear_color?: Color): number;
  draw_list_begin(framebuffer: RID, draw_flags?: number, clear_color_values?: PackedColorArray, clear_depth_value?: number, clear_stencil_value?: number, region?: Rect2, breadcrumb?: number): number;
  draw_list_begin_split(framebuffer: RID, splits: number, initial_color_action: number, final_color_action: number, initial_depth_action: number, final_depth_action: number, clear_color_values?: PackedColorArray, clear_depth?: number, clear_stencil?: number, region?: Rect2, storage_textures?: any[]): PackedInt64Array;
  draw_list_set_blend_constants(draw_list: number, color: Color): void;
  draw_list_bind_render_pipeline(draw_list: number, render_pipeline: RID): void;
  draw_list_bind_uniform_set(draw_list: number, uniform_set: RID, set_index: number): void;
  draw_list_bind_vertex_array(draw_list: number, vertex_array: RID): void;
  draw_list_bind_index_array(draw_list: number, index_array: RID): void;
  draw_list_set_push_constant(draw_list: number, buffer: PackedByteArray, size_bytes: number): void;
  draw_list_draw(draw_list: number, use_indices: boolean, instances: number, procedural_vertex_count?: number): void;
  draw_list_draw_indirect(draw_list: number, use_indices: boolean, buffer: RID, offset?: number, draw_count?: number, stride?: number): void;
  draw_list_enable_scissor(draw_list: number, rect?: Rect2): void;
  draw_list_disable_scissor(draw_list: number): void;
  draw_list_switch_to_next_pass(): number;
  draw_list_switch_to_next_pass_split(splits: number): PackedInt64Array;
  draw_list_end(): void;
  compute_list_begin(): number;
  compute_list_bind_compute_pipeline(compute_list: number, compute_pipeline: RID): void;
  compute_list_set_push_constant(compute_list: number, buffer: PackedByteArray, size_bytes: number): void;
  compute_list_bind_uniform_set(compute_list: number, uniform_set: RID, set_index: number): void;
  compute_list_dispatch(compute_list: number, x_groups: number, y_groups: number, z_groups: number): void;
  compute_list_dispatch_indirect(compute_list: number, buffer: RID, offset: number): void;
  compute_list_add_barrier(compute_list: number): void;
  compute_list_end(): void;
  free_rid(rid: RID): void;
  capture_timestamp(name: string): void;
  get_captured_timestamps_count(): number;
  get_captured_timestamps_frame(): number;
  get_captured_timestamp_gpu_time(index: number): number;
  get_captured_timestamp_cpu_time(index: number): number;
  get_captured_timestamp_name(index: number): string;
  has_feature(feature: number): boolean;
  limit_get(limit: number): number;
  get_frame_delay(): number;
  submit(): void;
  sync(): void;
  barrier(from?: number, to?: number): void;
  full_barrier(): void;
  create_local_device(): RenderingDevice;
  set_resource_name(id: RID, name: string): void;
  draw_command_begin_label(name: string, color: Color): void;
  draw_command_insert_label(name: string, color: Color): void;
  draw_command_end_label(): void;
  get_device_vendor_name(): string;
  get_device_name(): string;
  get_device_pipeline_cache_uuid(): string;
  get_memory_usage(type: number): number;
  get_driver_resource(resource: number, rid: RID, index: number): number;
  get_perf_report(): string;
  get_driver_and_device_memory_report(): string;
  get_tracked_object_name(type_index: number): string;
  get_tracked_object_type_count(): number;
  get_driver_total_memory(): number;
  get_driver_allocation_count(): number;
  get_driver_memory_by_object_type(type: number): number;
  get_driver_allocs_by_object_type(type: number): number;
  get_device_total_memory(): number;
  get_device_allocation_count(): number;
  get_device_memory_by_object_type(type: number): number;
  get_device_allocs_by_object_type(type: number): number;
}

export declare class Resource extends RefCounted {
  resource_local_to_scene: boolean;
  resource_path: string;
  resource_name: string;
  resource_scene_unique_id: string;
  take_over_path(path: string): void;
  set_path_cache(path: string): void;
  get_rid(): RID;
  get_local_scene(): Node;
  setup_local_to_scene(): void;
  reset_state(): void;
  set_id_for_path(path: string, id: string): void;
  get_id_for_path(path: string): string;
  is_built_in(): boolean;
  generate_scene_unique_id(): string;
  duplicate(deep?: boolean): Resource;
  duplicate_deep(deep_subresources_mode?: number): Resource;
}

export declare class ResourceFormatLoader extends RefCounted {
}

export declare class ResourceFormatSaver extends RefCounted {
}

export declare class ResourcePreloader extends Node {
  add_resource(name: string, resource: Resource): void;
  remove_resource(name: string): void;
  rename_resource(name: string, newname: string): void;
  has_resource(name: string): boolean;
  get_resource(name: string): Resource;
  get_resource_list(): PackedStringArray;
}

export declare class RetargetModifier3D extends SkeletonModifier3D {
  profile: SkeletonProfile;
  use_global_pose: boolean;
  enable: number;
  set_position_enabled(enabled: boolean): void;
  is_position_enabled(): boolean;
  set_rotation_enabled(enabled: boolean): void;
  is_rotation_enabled(): boolean;
  set_scale_enabled(enabled: boolean): void;
  is_scale_enabled(): boolean;
}

export declare class RibbonTrailMesh extends PrimitiveMesh {
  shape: number;
  size: number;
  sections: number;
  section_length: number;
  section_segments: number;
  curve: Curve;
}

export declare class RichTextEffect extends Resource {
}

export declare class RichTextLabel extends Control {
  bbcode_enabled: boolean;
  text: string;
  fit_content: boolean;
  scroll_active: boolean;
  scroll_following: boolean;
  scroll_following_visible_characters: boolean;
  autowrap_mode: number;
  autowrap_trim_flags: number;
  tab_size: number;
  context_menu_enabled: boolean;
  shortcut_keys_enabled: boolean;
  horizontal_alignment: number;
  vertical_alignment: number;
  justification_flags: number;
  tab_stops: PackedFloat32Array;
  meta_underlined: boolean;
  hint_underlined: boolean;
  threaded: boolean;
  progress_bar_delay: number;
  selection_enabled: boolean;
  deselect_on_focus_loss_enabled: boolean;
  drag_and_drop_selection_enabled: boolean;
  visible_characters: number;
  visible_characters_behavior: number;
  visible_ratio: number;
  text_direction: number;
  language: string;
  structured_text_bidi_override: number;
  structured_text_bidi_override_options: any[];
  get_parsed_text(): string;
  add_text(text: string): void;
  add_hr(width?: number, height?: number, color?: Color, alignment?: number, width_in_percent?: boolean, height_in_percent?: boolean): void;
  add_image(image: Texture2D, width?: number, height?: number, color?: Color, inline_align?: number, region?: Rect2, key?: any, pad?: boolean, tooltip?: string, width_in_percent?: boolean, height_in_percent?: boolean, alt_text?: string): void;
  update_image(key: any, mask: number, image: Texture2D, width?: number, height?: number, color?: Color, inline_align?: number, region?: Rect2, pad?: boolean, tooltip?: string, width_in_percent?: boolean, height_in_percent?: boolean): void;
  newline(): void;
  remove_paragraph(paragraph: number, no_invalidate?: boolean): boolean;
  invalidate_paragraph(paragraph: number): boolean;
  push_font(font: Font, font_size?: number): void;
  push_font_size(font_size: number): void;
  push_normal(): void;
  push_bold(): void;
  push_bold_italics(): void;
  push_italics(): void;
  push_mono(): void;
  push_color(color: Color): void;
  push_outline_size(outline_size: number): void;
  push_outline_color(color: Color): void;
  push_paragraph(alignment: number, base_direction?: number, language?: string, st_parser?: number, justification_flags?: number, tab_stops?: PackedFloat32Array): void;
  push_indent(level: number): void;
  push_list(level: number, type: number, capitalize: boolean, bullet?: string): void;
  push_meta(data: any, underline_mode?: number, tooltip?: string): void;
  push_hint(description: string): void;
  push_language(language: string): void;
  push_underline(color?: Color): void;
  push_strikethrough(color?: Color): void;
  push_table(columns: number, inline_align?: number, align_to_row?: number, name?: string): void;
  push_dropcap(string: string, font: Font, size: number, dropcap_margins?: Rect2, color?: Color, outline_size?: number, outline_color?: Color): void;
  set_table_column_expand(column: number, expand: boolean, ratio?: number, shrink?: boolean): void;
  set_table_column_name(column: number, name: string): void;
  set_cell_row_background_color(odd_row_bg: Color, even_row_bg: Color): void;
  set_cell_border_color(color: Color): void;
  set_cell_size_override(min_size: Vector2, max_size: Vector2): void;
  set_cell_padding(padding: Rect2): void;
  push_cell(): void;
  push_fgcolor(fgcolor: Color): void;
  push_bgcolor(bgcolor: Color): void;
  push_customfx(effect: RichTextEffect, env: Record<string, any>): void;
  push_context(): void;
  pop_context(): void;
  pop(): void;
  pop_all(): void;
  clear(): void;
  get_v_scroll_bar(): VScrollBar;
  scroll_to_line(line: number): void;
  scroll_to_paragraph(paragraph: number): void;
  scroll_to_selection(): void;
  get_selection_from(): number;
  get_selection_to(): number;
  get_selection_line_offset(): number;
  select_all(): void;
  get_selected_text(): string;
  deselect(): void;
  parse_bbcode(bbcode: string): void;
  append_text(bbcode: string): void;
  is_ready(): boolean;
  is_finished(): boolean;
  get_character_line(character: number): number;
  get_character_paragraph(character: number): number;
  get_total_character_count(): number;
  get_line_count(): number;
  get_line_range(line: number): Vector2i;
  get_visible_line_count(): number;
  get_paragraph_count(): number;
  get_visible_paragraph_count(): number;
  get_content_height(): number;
  get_content_width(): number;
  get_line_height(line: number): number;
  get_line_width(line: number): number;
  get_visible_content_rect(): Rect2i;
  get_line_offset(line: number): number;
  get_paragraph_offset(paragraph: number): number;
  parse_expressions_for_values(expressions: PackedStringArray): Record<string, any>;
  install_effect(effect: any): void;
  reload_effects(): void;
  get_menu(): PopupMenu;
  is_menu_visible(): boolean;
  menu_option(option: number): void;
}

export declare class RigidBody2D extends PhysicsBody2D {
  mass: number;
  physics_material_override: PhysicsMaterial;
  gravity_scale: number;
  center_of_mass_mode: number;
  center_of_mass: Vector2;
  inertia: number;
  sleeping: boolean;
  can_sleep: boolean;
  lock_rotation: boolean;
  freeze: boolean;
  freeze_mode: number;
  custom_integrator: boolean;
  continuous_cd: number;
  contact_monitor: boolean;
  max_contacts_reported: number;
  linear_velocity: Vector2;
  linear_damp_mode: number;
  linear_damp: number;
  angular_velocity: number;
  angular_damp_mode: number;
  angular_damp: number;
  constant_force: Vector2;
  constant_torque: number;
  get_contact_count(): number;
  set_axis_velocity(axis_velocity: Vector2): void;
  apply_central_impulse(impulse?: Vector2): void;
  apply_impulse(impulse: Vector2, position?: Vector2): void;
  apply_torque_impulse(torque: number): void;
  apply_central_force(force: Vector2): void;
  apply_force(force: Vector2, position?: Vector2): void;
  apply_torque(torque: number): void;
  add_constant_central_force(force: Vector2): void;
  add_constant_force(force: Vector2, position?: Vector2): void;
  add_constant_torque(torque: number): void;
  get_colliding_bodies(): any[];
}

export declare class RigidBody3D extends PhysicsBody3D {
  mass: number;
  physics_material_override: PhysicsMaterial;
  gravity_scale: number;
  center_of_mass_mode: number;
  center_of_mass: Vector3;
  inertia: Vector3;
  sleeping: boolean;
  can_sleep: boolean;
  lock_rotation: boolean;
  freeze: boolean;
  freeze_mode: number;
  custom_integrator: boolean;
  continuous_cd: boolean;
  contact_monitor: boolean;
  max_contacts_reported: number;
  linear_velocity: Vector3;
  linear_damp_mode: number;
  linear_damp: number;
  angular_velocity: Vector3;
  angular_damp_mode: number;
  angular_damp: number;
  constant_force: Vector3;
  constant_torque: Vector3;
  get_inverse_inertia_tensor(): Basis;
  get_contact_count(): number;
  set_axis_velocity(axis_velocity: Vector3): void;
  apply_central_impulse(impulse: Vector3): void;
  apply_impulse(impulse: Vector3, position?: Vector3): void;
  apply_torque_impulse(impulse: Vector3): void;
  apply_central_force(force: Vector3): void;
  apply_force(force: Vector3, position?: Vector3): void;
  apply_torque(torque: Vector3): void;
  add_constant_central_force(force: Vector3): void;
  add_constant_force(force: Vector3, position?: Vector3): void;
  add_constant_torque(torque: Vector3): void;
  get_colliding_bodies(): any[];
}

export declare class RootMotionView extends VisualInstance3D {
  animation_path: string;
  color: Color;
  cell_size: number;
  radius: number;
  zero_y: boolean;
}

export declare class SceneMultiplayer extends MultiplayerAPI {
  root_path: string;
  auth_callback: Callable;
  auth_timeout: number;
  allow_object_decoding: boolean;
  refuse_new_connections: boolean;
  server_relay: boolean;
  max_sync_packet_size: number;
  max_delta_packet_size: number;
  clear(): void;
  disconnect_peer(id: number): void;
  get_authenticating_peers(): PackedInt32Array;
  send_auth(id: number, data: PackedByteArray): number;
  complete_auth(id: number): number;
  send_bytes(bytes: PackedByteArray, id?: number, mode?: number, channel?: number): number;
}

export declare class SceneReplicationConfig extends Resource {
  get_properties(): any[];
  add_property(path: string, index?: number): void;
  has_property(path: string): boolean;
  remove_property(path: string): void;
  property_get_index(path: string): number;
  property_get_spawn(path: string): boolean;
  property_set_spawn(path: string, enabled: boolean): void;
  property_get_replication_mode(path: string): number;
  property_set_replication_mode(path: string, mode: number): void;
  property_get_sync(path: string): boolean;
  property_set_sync(path: string, enabled: boolean): void;
  property_get_watch(path: string): boolean;
  property_set_watch(path: string, enabled: boolean): void;
}

export declare class SceneState extends RefCounted {
  get_path(): string;
  get_base_scene_state(): SceneState;
  get_node_count(): number;
  get_node_type(idx: number): string;
  get_node_name(idx: number): string;
  get_node_path(idx: number, for_parent?: boolean): string;
  get_node_owner_path(idx: number): string;
  is_node_instance_placeholder(idx: number): boolean;
  get_node_instance_placeholder(idx: number): string;
  get_node_instance(idx: number): PackedScene;
  get_node_groups(idx: number): PackedStringArray;
  get_node_index(idx: number): number;
  get_node_property_count(idx: number): number;
  get_node_property_name(idx: number, prop_idx: number): string;
  get_node_property_value(idx: number, prop_idx: number): any;
  get_connection_count(): number;
  get_connection_source(idx: number): string;
  get_connection_signal(idx: number): string;
  get_connection_target(idx: number): string;
  get_connection_method(idx: number): string;
  get_connection_flags(idx: number): number;
  get_connection_binds(idx: number): any[];
  get_connection_unbinds(idx: number): number;
}

export declare class SceneTree extends MainLoop {
  auto_accept_quit: boolean;
  quit_on_go_back: boolean;
  debug_collisions_hint: boolean;
  debug_paths_hint: boolean;
  debug_navigation_hint: boolean;
  paused: boolean;
  edited_scene_root: Node;
  current_scene: Node;
  multiplayer_poll: boolean;
  physics_interpolation: boolean;
  has_group(name: string): boolean;
  is_accessibility_enabled(): boolean;
  is_accessibility_supported(): boolean;
  create_timer(time_sec: number, process_always?: boolean, process_in_physics?: boolean, ignore_time_scale?: boolean): SceneTreeTimer;
  create_tween(): Tween;
  get_processed_tweens(): any[];
  get_node_count(): number;
  get_frame(): number;
  quit(exit_code?: number): void;
  queue_delete(obj: any): void;
  notify_group_flags(call_flags: number, group: string, notification: number): void;
  set_group_flags(call_flags: number, group: string, property: string, value: any): void;
  notify_group(group: string, notification: number): void;
  set_group(group: string, property: string, value: any): void;
  get_nodes_in_group(group: string): any[];
  get_first_node_in_group(group: string): Node;
  get_node_count_in_group(group: string): number;
  change_scene_to_file(path: string): number;
  change_scene_to_packed(packed_scene: PackedScene): number;
  reload_current_scene(): number;
  unload_current_scene(): void;
  set_multiplayer(multiplayer: MultiplayerAPI, root_path?: string): void;
  get_multiplayer(for_path?: string): MultiplayerAPI;
}

export declare class SceneTreeTimer extends RefCounted {
  time_left: number;
}

export declare class Script extends Resource {
  source_code: string;
  can_instantiate(): boolean;
  instance_has(base_object: any): boolean;
  has_source_code(): boolean;
  reload(keep_state?: boolean): number;
  get_base_script(): Script;
  get_instance_base_type(): string;
  get_global_name(): string;
  has_script_signal(signal_name: string): boolean;
  get_script_property_list(): any[];
  get_script_method_list(): any[];
  get_script_signal_list(): any[];
  get_script_constant_map(): Record<string, any>;
  get_property_default_value(property: string): any;
  is_tool(): boolean;
  is_abstract(): boolean;
  get_rpc_config(): any;
}

export declare class ScriptBacktrace extends RefCounted {
  get_language_name(): string;
  is_empty(): boolean;
  get_frame_count(): number;
  get_frame_function(index: number): string;
  get_frame_file(index: number): string;
  get_frame_line(index: number): number;
  get_global_variable_count(): number;
  get_global_variable_name(variable_index: number): string;
  get_global_variable_value(variable_index: number): any;
  get_local_variable_count(frame_index: number): number;
  get_local_variable_name(frame_index: number, variable_index: number): string;
  get_local_variable_value(frame_index: number, variable_index: number): any;
  get_member_variable_count(frame_index: number): number;
  get_member_variable_name(frame_index: number, variable_index: number): string;
  get_member_variable_value(frame_index: number, variable_index: number): any;
  format(indent_all?: number, indent_frames?: number): string;
}

export declare class ScriptEditorBase extends VBoxContainer {
  get_base_editor(): Control;
  add_syntax_highlighter(highlighter: EditorSyntaxHighlighter): void;
}

export declare class ScriptExtension extends Script {
}

export declare class ScriptLanguageExtension extends ScriptLanguage {
}

export declare class ScrollBar extends Range {
  custom_step: number;
}

export declare class ScrollContainer extends Container {
  follow_focus: boolean;
  draw_focus_border: boolean;
  scroll_horizontal: number;
  scroll_vertical: number;
  scroll_horizontal_custom_step: number;
  scroll_vertical_custom_step: number;
  horizontal_scroll_mode: number;
  vertical_scroll_mode: number;
  scroll_deadzone: number;
  get_h_scroll_bar(): HScrollBar;
  get_v_scroll_bar(): VScrollBar;
  ensure_control_visible(control: Control): void;
}

export declare class SegmentShape2D extends Shape2D {
  a: Vector2;
  b: Vector2;
}

export declare class SeparationRayShape2D extends Shape2D {
  length: number;
  slide_on_slope: boolean;
}

export declare class SeparationRayShape3D extends Shape3D {
  length: number;
  slide_on_slope: boolean;
}

export declare class Separator extends Control {
}

export declare class Shader extends Resource {
  code: string;
  get_mode(): number;
  set_default_texture_parameter(name: string, texture: Texture, index?: number): void;
  get_default_texture_parameter(name: string, index?: number): Texture;
  get_shader_uniform_list(get_groups?: boolean): any[];
  inspect_native_shader_code(): void;
}

export declare class ShaderGlobalsOverride extends Node {
}

export declare class ShaderInclude extends Resource {
  code: string;
}

export declare class ShaderIncludeDB extends Object {
  list_built_in_include_files(): PackedStringArray;
  has_built_in_include_file(filename: string): boolean;
  get_built_in_include_file(filename: string): string;
}

export declare class ShaderMaterial extends Material {
  shader: Shader;
  set_shader_parameter(param: string, value: any): void;
  get_shader_parameter(param: string): any;
}

export declare class Shape2D extends Resource {
  custom_solver_bias: number;
  collide(local_xform: Transform2D, with_shape: Shape2D, shape_xform: Transform2D): boolean;
  collide_with_motion(local_xform: Transform2D, local_motion: Vector2, with_shape: Shape2D, shape_xform: Transform2D, shape_motion: Vector2): boolean;
  collide_and_get_contacts(local_xform: Transform2D, with_shape: Shape2D, shape_xform: Transform2D): PackedVector2Array;
  collide_with_motion_and_get_contacts(local_xform: Transform2D, local_motion: Vector2, with_shape: Shape2D, shape_xform: Transform2D, shape_motion: Vector2): PackedVector2Array;
  draw(canvas_item: RID, color: Color): void;
  get_rect(): Rect2;
}

export declare class Shape3D extends Resource {
  custom_solver_bias: number;
  margin: number;
  get_debug_mesh(): ArrayMesh;
}

export declare class ShapeCast2D extends Node2D {
  enabled: boolean;
  shape: Shape2D;
  exclude_parent: boolean;
  target_position: Vector2;
  margin: number;
  max_results: number;
  collision_mask: number;
  readonly collision_result: any[];
  collide_with_areas: boolean;
  collide_with_bodies: boolean;
  is_colliding(): boolean;
  get_collision_count(): number;
  force_shapecast_update(): void;
  get_collider(index: number): any;
  get_collider_rid(index: number): RID;
  get_collider_shape(index: number): number;
  get_collision_point(index: number): Vector2;
  get_collision_normal(index: number): Vector2;
  get_closest_collision_safe_fraction(): number;
  get_closest_collision_unsafe_fraction(): number;
  add_exception_rid(rid: RID): void;
  add_exception(node: CollisionObject2D): void;
  remove_exception_rid(rid: RID): void;
  remove_exception(node: CollisionObject2D): void;
  clear_exceptions(): void;
  set_collision_mask_value(layer_number: number, value: boolean): void;
  get_collision_mask_value(layer_number: number): boolean;
}

export declare class ShapeCast3D extends Node3D {
  enabled: boolean;
  shape: Shape3D;
  exclude_parent: boolean;
  target_position: Vector3;
  margin: number;
  max_results: number;
  collision_mask: number;
  readonly collision_result: any[];
  collide_with_areas: boolean;
  collide_with_bodies: boolean;
  debug_shape_custom_color: Color;
  resource_changed(resource: Resource): void;
  is_colliding(): boolean;
  get_collision_count(): number;
  force_shapecast_update(): void;
  get_collider(index: number): any;
  get_collider_rid(index: number): RID;
  get_collider_shape(index: number): number;
  get_collision_point(index: number): Vector3;
  get_collision_normal(index: number): Vector3;
  get_closest_collision_safe_fraction(): number;
  get_closest_collision_unsafe_fraction(): number;
  add_exception_rid(rid: RID): void;
  add_exception(node: CollisionObject3D): void;
  remove_exception_rid(rid: RID): void;
  remove_exception(node: CollisionObject3D): void;
  clear_exceptions(): void;
  set_collision_mask_value(layer_number: number, value: boolean): void;
  get_collision_mask_value(layer_number: number): boolean;
}

export declare class Shortcut extends Resource {
  has_valid_event(): boolean;
  matches_event(event: InputEvent): boolean;
  get_as_text(): string;
}

export declare class Skeleton2D extends Node2D {
  get_bone_count(): number;
  get_bone(idx: number): Bone2D;
  get_skeleton(): RID;
  set_modification_stack(modification_stack: SkeletonModificationStack2D): void;
  get_modification_stack(): SkeletonModificationStack2D;
  execute_modifications(delta: number, execution_mode: number): void;
  set_bone_local_pose_override(bone_idx: number, override_pose: Transform2D, strength: number, persistent: boolean): void;
  get_bone_local_pose_override(bone_idx: number): Transform2D;
}

export declare class Skeleton3D extends Node3D {
  motion_scale: number;
  show_rest_only: boolean;
  modifier_callback_mode_process: number;
  animate_physical_bones: boolean;
  add_bone(name: string): number;
  find_bone(name: string): number;
  get_bone_name(bone_idx: number): string;
  set_bone_name(bone_idx: number, name: string): void;
  get_bone_meta(bone_idx: number, key: string): any;
  get_bone_meta_list(bone_idx: number): any[];
  has_bone_meta(bone_idx: number, key: string): boolean;
  set_bone_meta(bone_idx: number, key: string, value: any): void;
  get_concatenated_bone_names(): string;
  get_bone_parent(bone_idx: number): number;
  set_bone_parent(bone_idx: number, parent_idx: number): void;
  get_bone_count(): number;
  get_version(): number;
  unparent_bone_and_rest(bone_idx: number): void;
  get_bone_children(bone_idx: number): PackedInt32Array;
  get_parentless_bones(): PackedInt32Array;
  get_bone_rest(bone_idx: number): Transform3D;
  set_bone_rest(bone_idx: number, rest: Transform3D): void;
  get_bone_global_rest(bone_idx: number): Transform3D;
  create_skin_from_rest_transforms(): Skin;
  register_skin(skin: Skin): SkinReference;
  localize_rests(): void;
  clear_bones(): void;
  get_bone_pose(bone_idx: number): Transform3D;
  set_bone_pose(bone_idx: number, pose: Transform3D): void;
  set_bone_pose_position(bone_idx: number, position: Vector3): void;
  set_bone_pose_rotation(bone_idx: number, rotation: Quaternion): void;
  set_bone_pose_scale(bone_idx: number, scale: Vector3): void;
  get_bone_pose_position(bone_idx: number): Vector3;
  get_bone_pose_rotation(bone_idx: number): Quaternion;
  get_bone_pose_scale(bone_idx: number): Vector3;
  reset_bone_pose(bone_idx: number): void;
  reset_bone_poses(): void;
  is_bone_enabled(bone_idx: number): boolean;
  set_bone_enabled(bone_idx: number, enabled?: boolean): void;
  get_bone_global_pose(bone_idx: number): Transform3D;
  set_bone_global_pose(bone_idx: number, pose: Transform3D): void;
  force_update_all_bone_transforms(): void;
  force_update_bone_child_transform(bone_idx: number): void;
  advance(delta: number): void;
  clear_bones_global_pose_override(): void;
  set_bone_global_pose_override(bone_idx: number, pose: Transform3D, amount: number, persistent?: boolean): void;
  get_bone_global_pose_override(bone_idx: number): Transform3D;
  get_bone_global_pose_no_override(bone_idx: number): Transform3D;
  physical_bones_stop_simulation(): void;
  physical_bones_start_simulation(bones?: any[]): void;
  physical_bones_add_collision_exception(exception: RID): void;
  physical_bones_remove_collision_exception(exception: RID): void;
}

export declare class SkeletonIK3D extends SkeletonModifier3D {
  root_bone: string;
  tip_bone: string;
  target: Transform3D;
  override_tip_basis: boolean;
  use_magnet: boolean;
  magnet: Vector3;
  target_node: string;
  min_distance: number;
  max_iterations: number;
  interpolation: number;
  get_parent_skeleton(): Skeleton3D;
  is_running(): boolean;
  start(one_time?: boolean): void;
  stop(): void;
}

export declare class SkeletonModification2D extends Resource {
  enabled: boolean;
  execution_mode: number;
  get_modification_stack(): SkeletonModificationStack2D;
  set_is_setup(is_setup: boolean): void;
  get_is_setup(): boolean;
  clamp_angle(angle: number, min: number, max: number, invert: boolean): number;
  set_editor_draw_gizmo(draw_gizmo: boolean): void;
  get_editor_draw_gizmo(): boolean;
}

export declare class SkeletonModification2DCCDIK extends SkeletonModification2D {
  target_nodepath: string;
  tip_nodepath: string;
  ccdik_data_chain_length: number;
  set_ccdik_joint_bone2d_node(joint_idx: number, bone2d_nodepath: string): void;
  get_ccdik_joint_bone2d_node(joint_idx: number): string;
  set_ccdik_joint_bone_index(joint_idx: number, bone_idx: number): void;
  get_ccdik_joint_bone_index(joint_idx: number): number;
  set_ccdik_joint_rotate_from_joint(joint_idx: number, rotate_from_joint: boolean): void;
  get_ccdik_joint_rotate_from_joint(joint_idx: number): boolean;
  set_ccdik_joint_enable_constraint(joint_idx: number, enable_constraint: boolean): void;
  get_ccdik_joint_enable_constraint(joint_idx: number): boolean;
  set_ccdik_joint_constraint_angle_min(joint_idx: number, angle_min: number): void;
  get_ccdik_joint_constraint_angle_min(joint_idx: number): number;
  set_ccdik_joint_constraint_angle_max(joint_idx: number, angle_max: number): void;
  get_ccdik_joint_constraint_angle_max(joint_idx: number): number;
  set_ccdik_joint_constraint_angle_invert(joint_idx: number, invert: boolean): void;
  get_ccdik_joint_constraint_angle_invert(joint_idx: number): boolean;
}

export declare class SkeletonModification2DFABRIK extends SkeletonModification2D {
  target_nodepath: string;
  fabrik_data_chain_length: number;
  set_fabrik_joint_bone2d_node(joint_idx: number, bone2d_nodepath: string): void;
  get_fabrik_joint_bone2d_node(joint_idx: number): string;
  set_fabrik_joint_bone_index(joint_idx: number, bone_idx: number): void;
  get_fabrik_joint_bone_index(joint_idx: number): number;
  set_fabrik_joint_magnet_position(joint_idx: number, magnet_position: Vector2): void;
  get_fabrik_joint_magnet_position(joint_idx: number): Vector2;
  set_fabrik_joint_use_target_rotation(joint_idx: number, use_target_rotation: boolean): void;
  get_fabrik_joint_use_target_rotation(joint_idx: number): boolean;
}

export declare class SkeletonModification2DJiggle extends SkeletonModification2D {
  target_nodepath: string;
  jiggle_data_chain_length: number;
  stiffness: number;
  mass: number;
  damping: number;
  use_gravity: boolean;
  gravity: Vector2;
  set_use_colliders(use_colliders: boolean): void;
  get_use_colliders(): boolean;
  set_collision_mask(collision_mask: number): void;
  get_collision_mask(): number;
  set_jiggle_joint_bone2d_node(joint_idx: number, bone2d_node: string): void;
  get_jiggle_joint_bone2d_node(joint_idx: number): string;
  set_jiggle_joint_bone_index(joint_idx: number, bone_idx: number): void;
  get_jiggle_joint_bone_index(joint_idx: number): number;
  set_jiggle_joint_override(joint_idx: number, override: boolean): void;
  get_jiggle_joint_override(joint_idx: number): boolean;
  set_jiggle_joint_stiffness(joint_idx: number, stiffness: number): void;
  get_jiggle_joint_stiffness(joint_idx: number): number;
  set_jiggle_joint_mass(joint_idx: number, mass: number): void;
  get_jiggle_joint_mass(joint_idx: number): number;
  set_jiggle_joint_damping(joint_idx: number, damping: number): void;
  get_jiggle_joint_damping(joint_idx: number): number;
  set_jiggle_joint_use_gravity(joint_idx: number, use_gravity: boolean): void;
  get_jiggle_joint_use_gravity(joint_idx: number): boolean;
  set_jiggle_joint_gravity(joint_idx: number, gravity: Vector2): void;
  get_jiggle_joint_gravity(joint_idx: number): Vector2;
}

export declare class SkeletonModification2DLookAt extends SkeletonModification2D {
  bone_index: number;
  bone2d_node: string;
  target_nodepath: string;
  set_additional_rotation(rotation: number): void;
  get_additional_rotation(): number;
  set_enable_constraint(enable_constraint: boolean): void;
  get_enable_constraint(): boolean;
  set_constraint_angle_min(angle_min: number): void;
  get_constraint_angle_min(): number;
  set_constraint_angle_max(angle_max: number): void;
  get_constraint_angle_max(): number;
  set_constraint_angle_invert(invert: boolean): void;
  get_constraint_angle_invert(): boolean;
}

export declare class SkeletonModification2DPhysicalBones extends SkeletonModification2D {
  physical_bone_chain_length: number;
  set_physical_bone_node(joint_idx: number, physicalbone2d_node: string): void;
  get_physical_bone_node(joint_idx: number): string;
  fetch_physical_bones(): void;
  start_simulation(bones?: any[]): void;
  stop_simulation(bones?: any[]): void;
}

export declare class SkeletonModification2DStackHolder extends SkeletonModification2D {
  set_held_modification_stack(held_modification_stack: SkeletonModificationStack2D): void;
  get_held_modification_stack(): SkeletonModificationStack2D;
}

export declare class SkeletonModification2DTwoBoneIK extends SkeletonModification2D {
  target_nodepath: string;
  target_minimum_distance: number;
  target_maximum_distance: number;
  flip_bend_direction: boolean;
  set_joint_one_bone2d_node(bone2d_node: string): void;
  get_joint_one_bone2d_node(): string;
  set_joint_one_bone_idx(bone_idx: number): void;
  get_joint_one_bone_idx(): number;
  set_joint_two_bone2d_node(bone2d_node: string): void;
  get_joint_two_bone2d_node(): string;
  set_joint_two_bone_idx(bone_idx: number): void;
  get_joint_two_bone_idx(): number;
}

export declare class SkeletonModificationStack2D extends Resource {
  enabled: boolean;
  strength: number;
  modification_count: number;
  setup(): void;
  execute(delta: number, execution_mode: number): void;
  enable_all_modifications(enabled: boolean): void;
  get_modification(mod_idx: number): SkeletonModification2D;
  add_modification(modification: SkeletonModification2D): void;
  delete_modification(mod_idx: number): void;
  set_modification(mod_idx: number, modification: SkeletonModification2D): void;
  get_is_setup(): boolean;
  get_skeleton(): Skeleton2D;
}

export declare class SkeletonModifier3D extends Node3D {
  active: boolean;
  influence: number;
  get_skeleton(): Skeleton3D;
}

export declare class SkeletonProfile extends Resource {
  root_bone: string;
  scale_base_bone: string;
  group_size: number;
  bone_size: number;
  get_group_name(group_idx: number): string;
  set_group_name(group_idx: number, group_name: string): void;
  get_texture(group_idx: number): Texture2D;
  set_texture(group_idx: number, texture: Texture2D): void;
  find_bone(bone_name: string): number;
  get_bone_name(bone_idx: number): string;
  set_bone_name(bone_idx: number, bone_name: string): void;
  get_bone_parent(bone_idx: number): string;
  set_bone_parent(bone_idx: number, bone_parent: string): void;
  get_tail_direction(bone_idx: number): number;
  set_tail_direction(bone_idx: number, tail_direction: number): void;
  get_bone_tail(bone_idx: number): string;
  set_bone_tail(bone_idx: number, bone_tail: string): void;
  get_reference_pose(bone_idx: number): Transform3D;
  set_reference_pose(bone_idx: number, bone_name: Transform3D): void;
  get_handle_offset(bone_idx: number): Vector2;
  set_handle_offset(bone_idx: number, handle_offset: Vector2): void;
  get_group(bone_idx: number): string;
  set_group(bone_idx: number, group: string): void;
  is_required(bone_idx: number): boolean;
  set_required(bone_idx: number, required: boolean): void;
}

export declare class SkeletonProfileHumanoid extends SkeletonProfile {
}

export declare class Skin extends Resource {
  set_bind_count(bind_count: number): void;
  get_bind_count(): number;
  add_bind(bone: number, pose: Transform3D): void;
  add_named_bind(name: string, pose: Transform3D): void;
  set_bind_pose(bind_index: number, pose: Transform3D): void;
  get_bind_pose(bind_index: number): Transform3D;
  set_bind_name(bind_index: number, name: string): void;
  get_bind_name(bind_index: number): string;
  set_bind_bone(bind_index: number, bone: number): void;
  get_bind_bone(bind_index: number): number;
  clear_binds(): void;
}

export declare class SkinReference extends RefCounted {
  get_skeleton(): RID;
  get_skin(): Skin;
}

export declare class Sky extends Resource {
  process_mode: number;
  radiance_size: number;
}

export declare class Slider extends Range {
  editable: boolean;
  scrollable: boolean;
  tick_count: number;
  ticks_on_borders: boolean;
  ticks_position: number;
}

export declare class SliderJoint3D extends Joint3D {
  set_param(param: number, value: number): void;
  get_param(param: number): number;
}

export declare class SoftBody3D extends MeshInstance3D {
  collision_layer: number;
  collision_mask: number;
  parent_collision_ignore: string;
  simulation_precision: number;
  total_mass: number;
  linear_stiffness: number;
  shrinking_factor: number;
  pressure_coefficient: number;
  damping_coefficient: number;
  drag_coefficient: number;
  ray_pickable: boolean;
  disable_mode: number;
  get_physics_rid(): RID;
  set_collision_mask_value(layer_number: number, value: boolean): void;
  get_collision_mask_value(layer_number: number): boolean;
  set_collision_layer_value(layer_number: number, value: boolean): void;
  get_collision_layer_value(layer_number: number): boolean;
  get_collision_exceptions(): any[];
  add_collision_exception_with(body: Node): void;
  remove_collision_exception_with(body: Node): void;
  get_point_transform(point_index: number): Vector3;
  apply_impulse(point_index: number, impulse: Vector3): void;
  apply_force(point_index: number, force: Vector3): void;
  apply_central_impulse(impulse: Vector3): void;
  apply_central_force(force: Vector3): void;
  set_point_pinned(point_index: number, pinned: boolean, attachment_path?: string, insert_at?: number): void;
  is_point_pinned(point_index: number): boolean;
}

export declare class SphereMesh extends PrimitiveMesh {
  radius: number;
  height: number;
  radial_segments: number;
  rings: number;
  is_hemisphere: boolean;
}

export declare class SphereOccluder3D extends Occluder3D {
  radius: number;
}

export declare class SphereShape3D extends Shape3D {
  radius: number;
}

export declare class SpinBox extends Range {
  alignment: number;
  editable: boolean;
  update_on_text_changed: boolean;
  prefix: string;
  suffix: string;
  custom_arrow_step: number;
  select_all_on_focus: boolean;
  apply(): void;
  get_line_edit(): LineEdit;
}

export declare class SplitContainer extends Container {
  split_offset: number;
  collapsed: boolean;
  dragging_enabled: boolean;
  dragger_visibility: number;
  vertical: boolean;
  touch_dragger_enabled: boolean;
  drag_area_margin_begin: number;
  drag_area_margin_end: number;
  drag_area_offset: number;
  drag_area_highlight_in_editor: boolean;
  clamp_split_offset(): void;
  get_drag_area_control(): Control;
}

export declare class SpotLight3D extends Light3D {
}

export declare class SpringArm3D extends Node3D {
  collision_mask: number;
  shape: Shape3D;
  spring_length: number;
  margin: number;
  get_hit_length(): number;
  add_excluded_object(RID: RID): void;
  remove_excluded_object(RID: RID): boolean;
  clear_excluded_objects(): void;
}

export declare class SpringBoneCollision3D extends Node3D {
  bone: number;
  position_offset: Vector3;
  rotation_offset: Quaternion;
  get_skeleton(): Skeleton3D;
}

export declare class SpringBoneCollisionCapsule3D extends SpringBoneCollision3D {
  radius: number;
  height: number;
  mid_height: number;
  inside: boolean;
}

export declare class SpringBoneCollisionPlane3D extends SpringBoneCollision3D {
}

export declare class SpringBoneCollisionSphere3D extends SpringBoneCollision3D {
  radius: number;
  inside: boolean;
}

export declare class SpringBoneSimulator3D extends SkeletonModifier3D {
  external_force: Vector3;
  setting_count: number;
  set_root_bone_name(index: number, bone_name: string): void;
  get_root_bone_name(index: number): string;
  set_root_bone(index: number, bone: number): void;
  get_root_bone(index: number): number;
  set_end_bone_name(index: number, bone_name: string): void;
  get_end_bone_name(index: number): string;
  set_end_bone(index: number, bone: number): void;
  get_end_bone(index: number): number;
  set_extend_end_bone(index: number, enabled: boolean): void;
  is_end_bone_extended(index: number): boolean;
  set_end_bone_direction(index: number, bone_direction: number): void;
  get_end_bone_direction(index: number): number;
  set_end_bone_length(index: number, length: number): void;
  get_end_bone_length(index: number): number;
  set_center_from(index: number, center_from: number): void;
  get_center_from(index: number): number;
  set_center_node(index: number, node_path: string): void;
  get_center_node(index: number): string;
  set_center_bone_name(index: number, bone_name: string): void;
  get_center_bone_name(index: number): string;
  set_center_bone(index: number, bone: number): void;
  get_center_bone(index: number): number;
  set_radius(index: number, radius: number): void;
  get_radius(index: number): number;
  set_rotation_axis(index: number, axis: number): void;
  get_rotation_axis(index: number): number;
  set_rotation_axis_vector(index: number, vector: Vector3): void;
  get_rotation_axis_vector(index: number): Vector3;
  set_radius_damping_curve(index: number, curve: Curve): void;
  get_radius_damping_curve(index: number): Curve;
  set_stiffness(index: number, stiffness: number): void;
  get_stiffness(index: number): number;
  set_stiffness_damping_curve(index: number, curve: Curve): void;
  get_stiffness_damping_curve(index: number): Curve;
  set_drag(index: number, drag: number): void;
  get_drag(index: number): number;
  set_drag_damping_curve(index: number, curve: Curve): void;
  get_drag_damping_curve(index: number): Curve;
  set_gravity(index: number, gravity: number): void;
  get_gravity(index: number): number;
  set_gravity_damping_curve(index: number, curve: Curve): void;
  get_gravity_damping_curve(index: number): Curve;
  set_gravity_direction(index: number, gravity_direction: Vector3): void;
  get_gravity_direction(index: number): Vector3;
  clear_settings(): void;
  set_individual_config(index: number, enabled: boolean): void;
  is_config_individual(index: number): boolean;
  get_joint_bone_name(index: number, joint: number): string;
  get_joint_bone(index: number, joint: number): number;
  set_joint_rotation_axis(index: number, joint: number, axis: number): void;
  get_joint_rotation_axis(index: number, joint: number): number;
  set_joint_rotation_axis_vector(index: number, joint: number, vector: Vector3): void;
  get_joint_rotation_axis_vector(index: number, joint: number): Vector3;
  set_joint_radius(index: number, joint: number, radius: number): void;
  get_joint_radius(index: number, joint: number): number;
  set_joint_stiffness(index: number, joint: number, stiffness: number): void;
  get_joint_stiffness(index: number, joint: number): number;
  set_joint_drag(index: number, joint: number, drag: number): void;
  get_joint_drag(index: number, joint: number): number;
  set_joint_gravity(index: number, joint: number, gravity: number): void;
  get_joint_gravity(index: number, joint: number): number;
  set_joint_gravity_direction(index: number, joint: number, gravity_direction: Vector3): void;
  get_joint_gravity_direction(index: number, joint: number): Vector3;
  get_joint_count(index: number): number;
  set_enable_all_child_collisions(index: number, enabled: boolean): void;
  are_all_child_collisions_enabled(index: number): boolean;
  set_exclude_collision_path(index: number, collision: number, node_path: string): void;
  get_exclude_collision_path(index: number, collision: number): string;
  set_exclude_collision_count(index: number, count: number): void;
  get_exclude_collision_count(index: number): number;
  clear_exclude_collisions(index: number): void;
  set_collision_path(index: number, collision: number, node_path: string): void;
  get_collision_path(index: number, collision: number): string;
  set_collision_count(index: number, count: number): void;
  get_collision_count(index: number): number;
  clear_collisions(index: number): void;
  reset(): void;
}

export declare class Sprite2D extends Node2D {
  texture: Texture2D;
  centered: boolean;
  offset: Vector2;
  flip_h: boolean;
  flip_v: boolean;
  hframes: number;
  vframes: number;
  frame: number;
  frame_coords: Vector2i;
  region_enabled: boolean;
  region_rect: Rect2;
  region_filter_clip_enabled: boolean;
  is_pixel_opaque(pos: Vector2): boolean;
  get_rect(): Rect2;
}

export declare class Sprite3D extends SpriteBase3D {
  texture: Texture2D;
  hframes: number;
  vframes: number;
  frame: number;
  frame_coords: Vector2i;
  region_enabled: boolean;
  region_rect: Rect2;
}

export declare class SpriteBase3D extends GeometryInstance3D {
  centered: boolean;
  offset: Vector2;
  flip_h: boolean;
  flip_v: boolean;
  modulate: Color;
  pixel_size: number;
  axis: number;
  billboard: number;
  alpha_cut: number;
  alpha_scissor_threshold: number;
  alpha_hash_scale: number;
  alpha_antialiasing_mode: number;
  alpha_antialiasing_edge: number;
  texture_filter: number;
  render_priority: number;
  get_item_rect(): Rect2;
  generate_triangle_mesh(): TriangleMesh;
}

export declare class SpriteFrames extends Resource {
  add_animation(anim: string): void;
  has_animation(anim: string): boolean;
  duplicate_animation(anim_from: string, anim_to: string): void;
  remove_animation(anim: string): void;
  rename_animation(anim: string, newname: string): void;
  get_animation_names(): PackedStringArray;
  set_animation_speed(anim: string, fps: number): void;
  get_animation_speed(anim: string): number;
  set_animation_loop(anim: string, loop: boolean): void;
  get_animation_loop(anim: string): boolean;
  add_frame(anim: string, texture: Texture2D, duration?: number, at_position?: number): void;
  set_frame(anim: string, idx: number, texture: Texture2D, duration?: number): void;
  remove_frame(anim: string, idx: number): void;
  get_frame_count(anim: string): number;
  get_frame_texture(anim: string, idx: number): Texture2D;
  get_frame_duration(anim: string, idx: number): number;
  clear(anim: string): void;
  clear_all(): void;
}

export declare class StandardMaterial3D extends BaseMaterial3D {
}

export declare class StaticBody2D extends PhysicsBody2D {
  physics_material_override: PhysicsMaterial;
  constant_linear_velocity: Vector2;
  constant_angular_velocity: number;
}

export declare class StaticBody3D extends PhysicsBody3D {
  physics_material_override: PhysicsMaterial;
  constant_linear_velocity: Vector3;
  constant_angular_velocity: Vector3;
}

export declare class StatusIndicator extends Node {
  tooltip: string;
  icon: Texture2D;
  menu: string;
  visible: boolean;
  get_rect(): Rect2;
}

export declare class StreamPeer extends RefCounted {
  big_endian: boolean;
  put_data(data: PackedByteArray): number;
  put_partial_data(data: PackedByteArray): any[];
  get_data(bytes: number): any[];
  get_partial_data(bytes: number): any[];
  get_available_bytes(): number;
  put_8(value: number): void;
  put_u8(value: number): void;
  put_16(value: number): void;
  put_u16(value: number): void;
  put_32(value: number): void;
  put_u32(value: number): void;
  put_64(value: number): void;
  put_u64(value: number): void;
  put_half(value: number): void;
  put_float(value: number): void;
  put_double(value: number): void;
  put_string(value: string): void;
  put_utf8_string(value: string): void;
  put_var(value: any, full_objects?: boolean): void;
  get_8(): number;
  get_u8(): number;
  get_16(): number;
  get_u16(): number;
  get_32(): number;
  get_u32(): number;
  get_64(): number;
  get_u64(): number;
  get_half(): number;
  get_float(): number;
  get_double(): number;
  get_string(bytes?: number): string;
  get_utf8_string(bytes?: number): string;
  get_var(allow_objects?: boolean): any;
}

export declare class StreamPeerBuffer extends StreamPeer {
  data_array: PackedByteArray;
  seek(position: number): void;
  get_size(): number;
  get_position(): number;
  resize(size: number): void;
  clear(): void;
  duplicate(): StreamPeerBuffer;
}

export declare class StreamPeerExtension extends StreamPeer {
}

export declare class StreamPeerGZIP extends StreamPeer {
  start_compression(use_deflate?: boolean, buffer_size?: number): number;
  start_decompression(use_deflate?: boolean, buffer_size?: number): number;
  finish(): number;
  clear(): void;
}

export declare class StreamPeerTCP extends StreamPeer {
  bind(port: number, host?: string): number;
  connect_to_host(host: string, port: number): number;
  poll(): number;
  get_status(): number;
  get_connected_host(): string;
  get_connected_port(): number;
  get_local_port(): number;
  disconnect_from_host(): void;
  set_no_delay(enabled: boolean): void;
}

export declare class StreamPeerTLS extends StreamPeer {
  poll(): void;
  accept_stream(stream: StreamPeer, server_options: TLSOptions): number;
  connect_to_stream(stream: StreamPeer, common_name: string, client_options?: TLSOptions): number;
  get_status(): number;
  get_stream(): StreamPeer;
  disconnect_from_stream(): void;
}

export declare class StyleBox extends Resource {
  get_minimum_size(): Vector2;
  set_content_margin_all(offset: number): void;
  get_margin(margin: number): number;
  get_offset(): Vector2;
  draw(canvas_item: RID, rect: Rect2): void;
  get_current_item_drawn(): CanvasItem;
  test_mask(point: Vector2, rect: Rect2): boolean;
}

export declare class StyleBoxEmpty extends StyleBox {
}

export declare class StyleBoxFlat extends StyleBox {
  bg_color: Color;
  draw_center: boolean;
  skew: Vector2;
  border_color: Color;
  border_blend: boolean;
  corner_detail: number;
  shadow_color: Color;
  shadow_size: number;
  shadow_offset: Vector2;
  anti_aliasing: boolean;
  anti_aliasing_size: number;
  set_border_width_all(width: number): void;
  get_border_width_min(): number;
  set_corner_radius_all(radius: number): void;
  set_expand_margin_all(size: number): void;
}

export declare class StyleBoxLine extends StyleBox {
  color: Color;
  grow_begin: number;
  grow_end: number;
  thickness: number;
  vertical: boolean;
}

export declare class StyleBoxTexture extends StyleBox {
  texture: Texture2D;
  axis_stretch_horizontal: number;
  axis_stretch_vertical: number;
  region_rect: Rect2;
  modulate_color: Color;
  draw_center: boolean;
  set_texture_margin_all(size: number): void;
  set_expand_margin_all(size: number): void;
}

export declare class SubViewport extends Viewport {
  size: Vector2i;
  size_2d_override: Vector2i;
  size_2d_override_stretch: boolean;
  render_target_clear_mode: number;
  render_target_update_mode: number;
}

export declare class SubViewportContainer extends Container {
  stretch: boolean;
  stretch_shrink: number;
  mouse_target: boolean;
}

export declare class SubtweenTweener extends Tweener {
  set_delay(delay: number): SubtweenTweener;
}

export declare class SurfaceTool extends RefCounted {
  set_skin_weight_count(count: number): void;
  get_skin_weight_count(): number;
  set_custom_format(channel_index: number, format: number): void;
  get_custom_format(channel_index: number): number;
  begin(primitive: number): void;
  add_vertex(vertex: Vector3): void;
  set_color(color: Color): void;
  set_normal(normal: Vector3): void;
  set_tangent(tangent: Plane): void;
  set_uv(uv: Vector2): void;
  set_uv2(uv2: Vector2): void;
  set_bones(bones: PackedInt32Array): void;
  set_weights(weights: PackedFloat32Array): void;
  set_custom(channel_index: number, custom_color: Color): void;
  set_smooth_group(index: number): void;
  add_triangle_fan(vertices: PackedVector3Array, uvs?: PackedVector2Array, colors?: PackedColorArray, uv2s?: PackedVector2Array, normals?: PackedVector3Array, tangents?: any[]): void;
  add_index(index: number): void;
  index(): void;
  deindex(): void;
  generate_normals(flip?: boolean): void;
  generate_tangents(): void;
  optimize_indices_for_cache(): void;
  get_aabb(): AABB;
  generate_lod(nd_threshold: number, target_index_count?: number): PackedInt32Array;
  set_material(material: Material): void;
  get_primitive_type(): number;
  clear(): void;
  create_from(existing: Mesh, surface: number): void;
  create_from_arrays(arrays: any[], primitive_type?: number): void;
  create_from_blend_shape(existing: Mesh, surface: number, blend_shape: string): void;
  append_from(existing: Mesh, surface: number, transform: Transform3D): void;
  commit(existing?: ArrayMesh, flags?: number): ArrayMesh;
  commit_to_arrays(): any[];
}

export declare class SyntaxHighlighter extends Resource {
  get_line_syntax_highlighting(line: number): Record<string, any>;
  update_cache(): void;
  clear_highlighting_cache(): void;
  get_text_edit(): TextEdit;
}

export declare class SystemFont extends Font {
  font_names: PackedStringArray;
  font_italic: boolean;
  font_weight: number;
  font_stretch: number;
  antialiasing: number;
  generate_mipmaps: boolean;
  disable_embedded_bitmaps: boolean;
  allow_system_fallback: boolean;
  force_autohinter: boolean;
  modulate_color_glyphs: boolean;
  hinting: number;
  subpixel_positioning: number;
  keep_rounding_remainders: boolean;
  multichannel_signed_distance_field: boolean;
  msdf_pixel_range: number;
  msdf_size: number;
  oversampling: number;
}

export declare class TLSOptions extends RefCounted {
  client(trusted_chain?: X509Certificate, common_name_override?: string): TLSOptions;
  client_unsafe(trusted_chain?: X509Certificate): TLSOptions;
  server(key: CryptoKey, certificate: X509Certificate): TLSOptions;
  is_server(): boolean;
  is_unsafe_client(): boolean;
  get_common_name_override(): string;
  get_trusted_ca_chain(): X509Certificate;
  get_private_key(): CryptoKey;
  get_own_certificate(): X509Certificate;
}

export declare class TabBar extends Control {
  current_tab: number;
  tab_alignment: number;
  clip_tabs: boolean;
  close_with_middle_mouse: boolean;
  tab_close_display_policy: number;
  max_tab_width: number;
  scrolling_enabled: boolean;
  drag_to_rearrange_enabled: boolean;
  tabs_rearrange_group: number;
  scroll_to_selected: boolean;
  select_with_rmb: boolean;
  deselect_enabled: boolean;
  tab_count: number;
  get_previous_tab(): number;
  select_previous_available(): boolean;
  select_next_available(): boolean;
  set_tab_title(tab_idx: number, title: string): void;
  get_tab_title(tab_idx: number): string;
  set_tab_tooltip(tab_idx: number, tooltip: string): void;
  get_tab_tooltip(tab_idx: number): string;
  set_tab_text_direction(tab_idx: number, direction: number): void;
  get_tab_text_direction(tab_idx: number): number;
  set_tab_language(tab_idx: number, language: string): void;
  get_tab_language(tab_idx: number): string;
  set_tab_icon(tab_idx: number, icon: Texture2D): void;
  get_tab_icon(tab_idx: number): Texture2D;
  set_tab_icon_max_width(tab_idx: number, width: number): void;
  get_tab_icon_max_width(tab_idx: number): number;
  set_tab_button_icon(tab_idx: number, icon: Texture2D): void;
  get_tab_button_icon(tab_idx: number): Texture2D;
  set_tab_disabled(tab_idx: number, disabled: boolean): void;
  is_tab_disabled(tab_idx: number): boolean;
  set_tab_hidden(tab_idx: number, hidden: boolean): void;
  is_tab_hidden(tab_idx: number): boolean;
  set_tab_metadata(tab_idx: number, metadata: any): void;
  get_tab_metadata(tab_idx: number): any;
  remove_tab(tab_idx: number): void;
  add_tab(title?: string, icon?: Texture2D): void;
  get_tab_idx_at_point(point: Vector2): number;
  get_tab_offset(): number;
  get_offset_buttons_visible(): boolean;
  ensure_tab_visible(idx: number): void;
  get_tab_rect(tab_idx: number): Rect2;
  move_tab(from: number, to: number): void;
  clear_tabs(): void;
}

export declare class TabContainer extends Container {
  tab_alignment: number;
  current_tab: number;
  tabs_position: number;
  clip_tabs: boolean;
  tabs_visible: boolean;
  all_tabs_in_front: boolean;
  drag_to_rearrange_enabled: boolean;
  tabs_rearrange_group: number;
  use_hidden_tabs_for_min_size: boolean;
  tab_focus_mode: number;
  deselect_enabled: boolean;
  get_tab_count(): number;
  get_previous_tab(): number;
  select_previous_available(): boolean;
  select_next_available(): boolean;
  get_current_tab_control(): Control;
  get_tab_bar(): TabBar;
  get_tab_control(tab_idx: number): Control;
  set_tab_title(tab_idx: number, title: string): void;
  get_tab_title(tab_idx: number): string;
  set_tab_tooltip(tab_idx: number, tooltip: string): void;
  get_tab_tooltip(tab_idx: number): string;
  set_tab_icon(tab_idx: number, icon: Texture2D): void;
  get_tab_icon(tab_idx: number): Texture2D;
  set_tab_icon_max_width(tab_idx: number, width: number): void;
  get_tab_icon_max_width(tab_idx: number): number;
  set_tab_disabled(tab_idx: number, disabled: boolean): void;
  is_tab_disabled(tab_idx: number): boolean;
  set_tab_hidden(tab_idx: number, hidden: boolean): void;
  is_tab_hidden(tab_idx: number): boolean;
  set_tab_metadata(tab_idx: number, metadata: any): void;
  get_tab_metadata(tab_idx: number): any;
  set_tab_button_icon(tab_idx: number, icon: Texture2D): void;
  get_tab_button_icon(tab_idx: number): Texture2D;
  get_tab_idx_at_point(point: Vector2): number;
  get_tab_idx_from_control(control: Control): number;
  set_popup(popup: Node): void;
  get_popup(): Popup;
}

export declare class TextEdit extends Control {
  text: string;
  placeholder_text: string;
  editable: boolean;
  context_menu_enabled: boolean;
  emoji_menu_enabled: boolean;
  backspace_deletes_composite_character_enabled: boolean;
  shortcut_keys_enabled: boolean;
  selecting_enabled: boolean;
  deselect_on_focus_loss_enabled: boolean;
  drag_and_drop_selection_enabled: boolean;
  virtual_keyboard_enabled: boolean;
  virtual_keyboard_show_on_focus: boolean;
  middle_mouse_paste_enabled: boolean;
  empty_selection_clipboard_enabled: boolean;
  wrap_mode: number;
  autowrap_mode: number;
  indent_wrapped_lines: boolean;
  tab_input_mode: boolean;
  scroll_smooth: boolean;
  scroll_v_scroll_speed: number;
  scroll_past_end_of_file: boolean;
  scroll_vertical: number;
  scroll_horizontal: number;
  scroll_fit_content_height: boolean;
  scroll_fit_content_width: boolean;
  minimap_draw: boolean;
  minimap_width: number;
  caret_type: number;
  caret_blink: boolean;
  caret_blink_interval: number;
  caret_draw_when_editable_disabled: boolean;
  caret_move_on_right_click: boolean;
  caret_mid_grapheme: boolean;
  caret_multiple: boolean;
  use_default_word_separators: boolean;
  use_custom_word_separators: boolean;
  custom_word_separators: string;
  syntax_highlighter: SyntaxHighlighter;
  highlight_all_occurrences: boolean;
  highlight_current_line: boolean;
  draw_control_chars: boolean;
  draw_tabs: boolean;
  draw_spaces: boolean;
  text_direction: number;
  language: string;
  structured_text_bidi_override: number;
  structured_text_bidi_override_options: any[];
  has_ime_text(): boolean;
  cancel_ime(): void;
  apply_ime(): void;
  set_tab_size(size: number): void;
  get_tab_size(): number;
  set_overtype_mode_enabled(enabled: boolean): void;
  is_overtype_mode_enabled(): boolean;
  clear(): void;
  get_line_count(): number;
  set_line(line: number, new_text: string): void;
  get_line(line: number): string;
  get_line_with_ime(line: number): string;
  get_line_width(line: number, wrap_index?: number): number;
  get_line_height(): number;
  get_indent_level(line: number): number;
  get_first_non_whitespace_column(line: number): number;
  swap_lines(from_line: number, to_line: number): void;
  insert_line_at(line: number, text: string): void;
  remove_line_at(line: number, move_carets_down?: boolean): void;
  insert_text_at_caret(text: string, caret_index?: number): void;
  insert_text(text: string, line: number, column: number, before_selection_begin?: boolean, before_selection_end?: boolean): void;
  remove_text(from_line: number, from_column: number, to_line: number, to_column: number): void;
  get_last_unhidden_line(): number;
  get_next_visible_line_offset_from(line: number, visible_amount: number): number;
  get_next_visible_line_index_offset_from(line: number, wrap_index: number, visible_amount: number): Vector2i;
  backspace(caret_index?: number): void;
  cut(caret_index?: number): void;
  copy(caret_index?: number): void;
  paste(caret_index?: number): void;
  paste_primary_clipboard(caret_index?: number): void;
  start_action(action: number): void;
  end_action(): void;
  begin_complex_operation(): void;
  end_complex_operation(): void;
  has_undo(): boolean;
  has_redo(): boolean;
  undo(): void;
  redo(): void;
  clear_undo_history(): void;
  tag_saved_version(): void;
  get_version(): number;
  get_saved_version(): number;
  set_search_text(search_text: string): void;
  set_search_flags(flags: number): void;
  search(text: string, flags: number, from_line: number, from_column: number): Vector2i;
  set_tooltip_request_func(callback: Callable): void;
  get_local_mouse_pos(): Vector2;
  get_word_at_pos(position: Vector2): string;
  get_line_column_at_pos(position: Vector2i, clamp_line?: boolean, clamp_column?: boolean): Vector2i;
  get_pos_at_line_column(line: number, column: number): Vector2i;
  get_rect_at_line_column(line: number, column: number): Rect2i;
  get_minimap_line_at_pos(position: Vector2i): number;
  is_dragging_cursor(): boolean;
  is_mouse_over_selection(edges: boolean, caret_index?: number): boolean;
  add_caret(line: number, column: number): number;
  remove_caret(caret: number): void;
  remove_secondary_carets(): void;
  get_caret_count(): number;
  add_caret_at_carets(below: boolean): void;
  get_sorted_carets(include_ignored_carets?: boolean): PackedInt32Array;
  collapse_carets(from_line: number, from_column: number, to_line: number, to_column: number, inclusive?: boolean): void;
  merge_overlapping_carets(): void;
  begin_multicaret_edit(): void;
  end_multicaret_edit(): void;
  is_in_mulitcaret_edit(): boolean;
  multicaret_edit_ignore_caret(caret_index: number): boolean;
  is_caret_visible(caret_index?: number): boolean;
  get_caret_draw_pos(caret_index?: number): Vector2;
  set_caret_line(line: number, adjust_viewport?: boolean, can_be_hidden?: boolean, wrap_index?: number, caret_index?: number): void;
  get_caret_line(caret_index?: number): number;
  set_caret_column(column: number, adjust_viewport?: boolean, caret_index?: number): void;
  get_caret_column(caret_index?: number): number;
  get_next_composite_character_column(line: number, column: number): number;
  get_previous_composite_character_column(line: number, column: number): number;
  get_caret_wrap_index(caret_index?: number): number;
  get_word_under_caret(caret_index?: number): string;
  set_selection_mode(mode: number): void;
  get_selection_mode(): number;
  select_all(): void;
  select_word_under_caret(caret_index?: number): void;
  add_selection_for_next_occurrence(): void;
  skip_selection_for_next_occurrence(): void;
  select(origin_line: number, origin_column: number, caret_line: number, caret_column: number, caret_index?: number): void;
  has_selection(caret_index?: number): boolean;
  get_selected_text(caret_index?: number): string;
  get_selection_at_line_column(line: number, column: number, include_edges?: boolean, only_selections?: boolean): number;
  get_line_ranges_from_carets(only_selections?: boolean, merge_adjacent?: boolean): any[];
  get_selection_origin_line(caret_index?: number): number;
  get_selection_origin_column(caret_index?: number): number;
  set_selection_origin_line(line: number, can_be_hidden?: boolean, wrap_index?: number, caret_index?: number): void;
  set_selection_origin_column(column: number, caret_index?: number): void;
  get_selection_from_line(caret_index?: number): number;
  get_selection_from_column(caret_index?: number): number;
  get_selection_to_line(caret_index?: number): number;
  get_selection_to_column(caret_index?: number): number;
  is_caret_after_selection_origin(caret_index?: number): boolean;
  deselect(caret_index?: number): void;
  delete_selection(caret_index?: number): void;
  is_line_wrapped(line: number): boolean;
  get_line_wrap_count(line: number): number;
  get_line_wrap_index_at_column(line: number, column: number): number;
  get_line_wrapped_text(line: number): PackedStringArray;
  get_v_scroll_bar(): VScrollBar;
  get_h_scroll_bar(): HScrollBar;
  get_scroll_pos_for_line(line: number, wrap_index?: number): number;
  set_line_as_first_visible(line: number, wrap_index?: number): void;
  get_first_visible_line(): number;
  set_line_as_center_visible(line: number, wrap_index?: number): void;
  set_line_as_last_visible(line: number, wrap_index?: number): void;
  get_last_full_visible_line(): number;
  get_last_full_visible_line_wrap_index(): number;
  get_visible_line_count(): number;
  get_visible_line_count_in_range(from_line: number, to_line: number): number;
  get_total_visible_line_count(): number;
  adjust_viewport_to_caret(caret_index?: number): void;
  center_viewport_to_caret(caret_index?: number): void;
  get_minimap_visible_lines(): number;
  add_gutter(at?: number): void;
  remove_gutter(gutter: number): void;
  get_gutter_count(): number;
  set_gutter_name(gutter: number, name: string): void;
  get_gutter_name(gutter: number): string;
  set_gutter_type(gutter: number, type: number): void;
  get_gutter_type(gutter: number): number;
  set_gutter_width(gutter: number, width: number): void;
  get_gutter_width(gutter: number): number;
  set_gutter_draw(gutter: number, draw: boolean): void;
  is_gutter_drawn(gutter: number): boolean;
  set_gutter_clickable(gutter: number, clickable: boolean): void;
  is_gutter_clickable(gutter: number): boolean;
  set_gutter_overwritable(gutter: number, overwritable: boolean): void;
  is_gutter_overwritable(gutter: number): boolean;
  merge_gutters(from_line: number, to_line: number): void;
  set_gutter_custom_draw(column: number, draw_callback: Callable): void;
  get_total_gutter_width(): number;
  set_line_gutter_metadata(line: number, gutter: number, metadata: any): void;
  get_line_gutter_metadata(line: number, gutter: number): any;
  set_line_gutter_text(line: number, gutter: number, text: string): void;
  get_line_gutter_text(line: number, gutter: number): string;
  set_line_gutter_icon(line: number, gutter: number, icon: Texture2D): void;
  get_line_gutter_icon(line: number, gutter: number): Texture2D;
  set_line_gutter_item_color(line: number, gutter: number, color: Color): void;
  get_line_gutter_item_color(line: number, gutter: number): Color;
  set_line_gutter_clickable(line: number, gutter: number, clickable: boolean): void;
  is_line_gutter_clickable(line: number, gutter: number): boolean;
  set_line_background_color(line: number, color: Color): void;
  get_line_background_color(line: number): Color;
  get_menu(): PopupMenu;
  is_menu_visible(): boolean;
  menu_option(option: number): void;
  adjust_carets_after_edit(caret: number, from_line: number, from_col: number, to_line: number, to_col: number): void;
  get_caret_index_edit_order(): PackedInt32Array;
  get_selection_line(caret_index?: number): number;
  get_selection_column(caret_index?: number): number;
}

export declare class TextLine extends RefCounted {
  direction: number;
  orientation: number;
  preserve_invalid: boolean;
  preserve_control: boolean;
  width: number;
  alignment: number;
  flags: number;
  text_overrun_behavior: number;
  ellipsis_char: string;
  clear(): void;
  get_inferred_direction(): number;
  set_bidi_override(override: any[]): void;
  add_string(text: string, font: Font, font_size: number, language?: string, meta?: any): boolean;
  add_object(key: any, size: Vector2, inline_align?: number, length?: number, baseline?: number): boolean;
  resize_object(key: any, size: Vector2, inline_align?: number, baseline?: number): boolean;
  tab_align(tab_stops: PackedFloat32Array): void;
  get_objects(): any[];
  get_object_rect(key: any): Rect2;
  get_size(): Vector2;
  get_rid(): RID;
  get_line_ascent(): number;
  get_line_descent(): number;
  get_line_width(): number;
  get_line_underline_position(): number;
  get_line_underline_thickness(): number;
  draw(canvas: RID, pos: Vector2, color?: Color, oversampling?: number): void;
  draw_outline(canvas: RID, pos: Vector2, outline_size?: number, color?: Color, oversampling?: number): void;
  hit_test(coords: number): number;
}

export declare class TextMesh extends PrimitiveMesh {
  text: string;
  font: Font;
  font_size: number;
  horizontal_alignment: number;
  vertical_alignment: number;
  uppercase: boolean;
  line_spacing: number;
  autowrap_mode: number;
  justification_flags: number;
  pixel_size: number;
  curve_step: number;
  depth: number;
  width: number;
  offset: Vector2;
  text_direction: number;
  language: string;
  structured_text_bidi_override: number;
  structured_text_bidi_override_options: any[];
}

export declare class TextParagraph extends RefCounted {
  direction: number;
  custom_punctuation: string;
  orientation: number;
  preserve_invalid: boolean;
  preserve_control: boolean;
  alignment: number;
  break_flags: number;
  justification_flags: number;
  text_overrun_behavior: number;
  ellipsis_char: string;
  width: number;
  max_lines_visible: number;
  line_spacing: number;
  clear(): void;
  get_inferred_direction(): number;
  set_bidi_override(override: any[]): void;
  set_dropcap(text: string, font: Font, font_size: number, dropcap_margins?: Rect2, language?: string): boolean;
  clear_dropcap(): void;
  add_string(text: string, font: Font, font_size: number, language?: string, meta?: any): boolean;
  add_object(key: any, size: Vector2, inline_align?: number, length?: number, baseline?: number): boolean;
  resize_object(key: any, size: Vector2, inline_align?: number, baseline?: number): boolean;
  tab_align(tab_stops: PackedFloat32Array): void;
  get_non_wrapped_size(): Vector2;
  get_size(): Vector2;
  get_rid(): RID;
  get_line_rid(line: number): RID;
  get_dropcap_rid(): RID;
  get_range(): Vector2i;
  get_line_count(): number;
  get_line_objects(line: number): any[];
  get_line_object_rect(line: number, key: any): Rect2;
  get_line_size(line: number): Vector2;
  get_line_range(line: number): Vector2i;
  get_line_ascent(line: number): number;
  get_line_descent(line: number): number;
  get_line_width(line: number): number;
  get_line_underline_position(line: number): number;
  get_line_underline_thickness(line: number): number;
  get_dropcap_size(): Vector2;
  get_dropcap_lines(): number;
  draw(canvas: RID, pos: Vector2, color?: Color, dc_color?: Color, oversampling?: number): void;
  draw_outline(canvas: RID, pos: Vector2, outline_size?: number, color?: Color, dc_color?: Color, oversampling?: number): void;
  draw_line(canvas: RID, pos: Vector2, line: number, color?: Color, oversampling?: number): void;
  draw_line_outline(canvas: RID, pos: Vector2, line: number, outline_size?: number, color?: Color, oversampling?: number): void;
  draw_dropcap(canvas: RID, pos: Vector2, color?: Color, oversampling?: number): void;
  draw_dropcap_outline(canvas: RID, pos: Vector2, outline_size?: number, color?: Color, oversampling?: number): void;
  hit_test(coords: Vector2): number;
}

export declare class TextServer extends RefCounted {
  has_feature(feature: number): boolean;
  get_name(): string;
  get_features(): number;
  load_support_data(filename: string): boolean;
  get_support_data_filename(): string;
  get_support_data_info(): string;
  save_support_data(filename: string): boolean;
  get_support_data(): PackedByteArray;
  is_locale_right_to_left(locale: string): boolean;
  name_to_tag(name: string): number;
  tag_to_name(tag: number): string;
  has(rid: RID): boolean;
  free_rid(rid: RID): void;
  create_font(): RID;
  create_font_linked_variation(font_rid: RID): RID;
  font_set_data(font_rid: RID, data: PackedByteArray): void;
  font_set_face_index(font_rid: RID, face_index: number): void;
  font_get_face_index(font_rid: RID): number;
  font_get_face_count(font_rid: RID): number;
  font_set_style(font_rid: RID, style: number): void;
  font_get_style(font_rid: RID): number;
  font_set_name(font_rid: RID, name: string): void;
  font_get_name(font_rid: RID): string;
  font_get_ot_name_strings(font_rid: RID): Record<string, any>;
  font_set_style_name(font_rid: RID, name: string): void;
  font_get_style_name(font_rid: RID): string;
  font_set_weight(font_rid: RID, weight: number): void;
  font_get_weight(font_rid: RID): number;
  font_set_stretch(font_rid: RID, weight: number): void;
  font_get_stretch(font_rid: RID): number;
  font_set_antialiasing(font_rid: RID, antialiasing: number): void;
  font_get_antialiasing(font_rid: RID): number;
  font_set_disable_embedded_bitmaps(font_rid: RID, disable_embedded_bitmaps: boolean): void;
  font_get_disable_embedded_bitmaps(font_rid: RID): boolean;
  font_set_generate_mipmaps(font_rid: RID, generate_mipmaps: boolean): void;
  font_get_generate_mipmaps(font_rid: RID): boolean;
  font_set_multichannel_signed_distance_field(font_rid: RID, msdf: boolean): void;
  font_is_multichannel_signed_distance_field(font_rid: RID): boolean;
  font_set_msdf_pixel_range(font_rid: RID, msdf_pixel_range: number): void;
  font_get_msdf_pixel_range(font_rid: RID): number;
  font_set_msdf_size(font_rid: RID, msdf_size: number): void;
  font_get_msdf_size(font_rid: RID): number;
  font_set_fixed_size(font_rid: RID, fixed_size: number): void;
  font_get_fixed_size(font_rid: RID): number;
  font_set_fixed_size_scale_mode(font_rid: RID, fixed_size_scale_mode: number): void;
  font_get_fixed_size_scale_mode(font_rid: RID): number;
  font_set_allow_system_fallback(font_rid: RID, allow_system_fallback: boolean): void;
  font_is_allow_system_fallback(font_rid: RID): boolean;
  font_clear_system_fallback_cache(): void;
  font_set_force_autohinter(font_rid: RID, force_autohinter: boolean): void;
  font_is_force_autohinter(font_rid: RID): boolean;
  font_set_modulate_color_glyphs(font_rid: RID, force_autohinter: boolean): void;
  font_is_modulate_color_glyphs(font_rid: RID): boolean;
  font_set_hinting(font_rid: RID, hinting: number): void;
  font_get_hinting(font_rid: RID): number;
  font_set_subpixel_positioning(font_rid: RID, subpixel_positioning: number): void;
  font_get_subpixel_positioning(font_rid: RID): number;
  font_set_keep_rounding_remainders(font_rid: RID, keep_rounding_remainders: boolean): void;
  font_get_keep_rounding_remainders(font_rid: RID): boolean;
  font_set_embolden(font_rid: RID, strength: number): void;
  font_get_embolden(font_rid: RID): number;
  font_set_spacing(font_rid: RID, spacing: number, value: number): void;
  font_get_spacing(font_rid: RID, spacing: number): number;
  font_set_baseline_offset(font_rid: RID, baseline_offset: number): void;
  font_get_baseline_offset(font_rid: RID): number;
  font_set_transform(font_rid: RID, transform: Transform2D): void;
  font_get_transform(font_rid: RID): Transform2D;
  font_set_variation_coordinates(font_rid: RID, variation_coordinates: Record<string, any>): void;
  font_get_variation_coordinates(font_rid: RID): Record<string, any>;
  font_set_oversampling(font_rid: RID, oversampling: number): void;
  font_get_oversampling(font_rid: RID): number;
  font_get_size_cache_list(font_rid: RID): any[];
  font_clear_size_cache(font_rid: RID): void;
  font_remove_size_cache(font_rid: RID, size: Vector2i): void;
  font_get_size_cache_info(font_rid: RID): any[];
  font_set_ascent(font_rid: RID, size: number, ascent: number): void;
  font_get_ascent(font_rid: RID, size: number): number;
  font_set_descent(font_rid: RID, size: number, descent: number): void;
  font_get_descent(font_rid: RID, size: number): number;
  font_set_underline_position(font_rid: RID, size: number, underline_position: number): void;
  font_get_underline_position(font_rid: RID, size: number): number;
  font_set_underline_thickness(font_rid: RID, size: number, underline_thickness: number): void;
  font_get_underline_thickness(font_rid: RID, size: number): number;
  font_set_scale(font_rid: RID, size: number, scale: number): void;
  font_get_scale(font_rid: RID, size: number): number;
  font_get_texture_count(font_rid: RID, size: Vector2i): number;
  font_clear_textures(font_rid: RID, size: Vector2i): void;
  font_remove_texture(font_rid: RID, size: Vector2i, texture_index: number): void;
  font_set_texture_image(font_rid: RID, size: Vector2i, texture_index: number, image: Image): void;
  font_get_texture_image(font_rid: RID, size: Vector2i, texture_index: number): Image;
  font_set_texture_offsets(font_rid: RID, size: Vector2i, texture_index: number, offset: PackedInt32Array): void;
  font_get_texture_offsets(font_rid: RID, size: Vector2i, texture_index: number): PackedInt32Array;
  font_get_glyph_list(font_rid: RID, size: Vector2i): PackedInt32Array;
  font_clear_glyphs(font_rid: RID, size: Vector2i): void;
  font_remove_glyph(font_rid: RID, size: Vector2i, glyph: number): void;
  font_get_glyph_advance(font_rid: RID, size: number, glyph: number): Vector2;
  font_set_glyph_advance(font_rid: RID, size: number, glyph: number, advance: Vector2): void;
  font_get_glyph_offset(font_rid: RID, size: Vector2i, glyph: number): Vector2;
  font_set_glyph_offset(font_rid: RID, size: Vector2i, glyph: number, offset: Vector2): void;
  font_get_glyph_size(font_rid: RID, size: Vector2i, glyph: number): Vector2;
  font_set_glyph_size(font_rid: RID, size: Vector2i, glyph: number, gl_size: Vector2): void;
  font_get_glyph_uv_rect(font_rid: RID, size: Vector2i, glyph: number): Rect2;
  font_set_glyph_uv_rect(font_rid: RID, size: Vector2i, glyph: number, uv_rect: Rect2): void;
  font_get_glyph_texture_idx(font_rid: RID, size: Vector2i, glyph: number): number;
  font_set_glyph_texture_idx(font_rid: RID, size: Vector2i, glyph: number, texture_idx: number): void;
  font_get_glyph_texture_rid(font_rid: RID, size: Vector2i, glyph: number): RID;
  font_get_glyph_texture_size(font_rid: RID, size: Vector2i, glyph: number): Vector2;
  font_get_glyph_contours(font: RID, size: number, index: number): Record<string, any>;
  font_get_kerning_list(font_rid: RID, size: number): any[];
  font_clear_kerning_map(font_rid: RID, size: number): void;
  font_remove_kerning(font_rid: RID, size: number, glyph_pair: Vector2i): void;
  font_set_kerning(font_rid: RID, size: number, glyph_pair: Vector2i, kerning: Vector2): void;
  font_get_kerning(font_rid: RID, size: number, glyph_pair: Vector2i): Vector2;
  font_get_glyph_index(font_rid: RID, size: number, char: number, variation_selector: number): number;
  font_get_char_from_glyph_index(font_rid: RID, size: number, glyph_index: number): number;
  font_has_char(font_rid: RID, char: number): boolean;
  font_get_supported_chars(font_rid: RID): string;
  font_get_supported_glyphs(font_rid: RID): PackedInt32Array;
  font_render_range(font_rid: RID, size: Vector2i, start: number, end: number): void;
  font_render_glyph(font_rid: RID, size: Vector2i, index: number): void;
  font_draw_glyph(font_rid: RID, canvas: RID, size: number, pos: Vector2, index: number, color?: Color, oversampling?: number): void;
  font_draw_glyph_outline(font_rid: RID, canvas: RID, size: number, outline_size: number, pos: Vector2, index: number, color?: Color, oversampling?: number): void;
  font_is_language_supported(font_rid: RID, language: string): boolean;
  font_set_language_support_override(font_rid: RID, language: string, supported: boolean): void;
  font_get_language_support_override(font_rid: RID, language: string): boolean;
  font_remove_language_support_override(font_rid: RID, language: string): void;
  font_get_language_support_overrides(font_rid: RID): PackedStringArray;
  font_is_script_supported(font_rid: RID, script: string): boolean;
  font_set_script_support_override(font_rid: RID, script: string, supported: boolean): void;
  font_get_script_support_override(font_rid: RID, script: string): boolean;
  font_remove_script_support_override(font_rid: RID, script: string): void;
  font_get_script_support_overrides(font_rid: RID): PackedStringArray;
  font_set_opentype_feature_overrides(font_rid: RID, overrides: Record<string, any>): void;
  font_get_opentype_feature_overrides(font_rid: RID): Record<string, any>;
  font_supported_feature_list(font_rid: RID): Record<string, any>;
  font_supported_variation_list(font_rid: RID): Record<string, any>;
  font_get_global_oversampling(): number;
  font_set_global_oversampling(oversampling: number): void;
  get_hex_code_box_size(size: number, index: number): Vector2;
  draw_hex_code_box(canvas: RID, size: number, pos: Vector2, index: number, color: Color): void;
  create_shaped_text(direction?: number, orientation?: number): RID;
  shaped_text_clear(rid: RID): void;
  shaped_text_set_direction(shaped: RID, direction?: number): void;
  shaped_text_get_direction(shaped: RID): number;
  shaped_text_get_inferred_direction(shaped: RID): number;
  shaped_text_set_bidi_override(shaped: RID, override: any[]): void;
  shaped_text_set_custom_punctuation(shaped: RID, punct: string): void;
  shaped_text_get_custom_punctuation(shaped: RID): string;
  shaped_text_set_custom_ellipsis(shaped: RID, char: number): void;
  shaped_text_get_custom_ellipsis(shaped: RID): number;
  shaped_text_set_orientation(shaped: RID, orientation?: number): void;
  shaped_text_get_orientation(shaped: RID): number;
  shaped_text_set_preserve_invalid(shaped: RID, enabled: boolean): void;
  shaped_text_get_preserve_invalid(shaped: RID): boolean;
  shaped_text_set_preserve_control(shaped: RID, enabled: boolean): void;
  shaped_text_get_preserve_control(shaped: RID): boolean;
  shaped_text_set_spacing(shaped: RID, spacing: number, value: number): void;
  shaped_text_get_spacing(shaped: RID, spacing: number): number;
  shaped_text_add_string(shaped: RID, text: string, fonts: any[], size: number, opentype_features?: Record<string, any>, language?: string, meta?: any): boolean;
  shaped_text_add_object(shaped: RID, key: any, size: Vector2, inline_align?: number, length?: number, baseline?: number): boolean;
  shaped_text_resize_object(shaped: RID, key: any, size: Vector2, inline_align?: number, baseline?: number): boolean;
  shaped_get_text(shaped: RID): string;
  shaped_get_span_count(shaped: RID): number;
  shaped_get_span_meta(shaped: RID, index: number): any;
  shaped_get_span_embedded_object(shaped: RID, index: number): any;
  shaped_get_span_text(shaped: RID, index: number): string;
  shaped_get_span_object(shaped: RID, index: number): any;
  shaped_set_span_update_font(shaped: RID, index: number, fonts: any[], size: number, opentype_features?: Record<string, any>): void;
  shaped_get_run_count(shaped: RID): number;
  shaped_get_run_text(shaped: RID, index: number): string;
  shaped_get_run_range(shaped: RID, index: number): Vector2i;
  shaped_get_run_font_rid(shaped: RID, index: number): RID;
  shaped_get_run_font_size(shaped: RID, index: number): number;
  shaped_get_run_language(shaped: RID, index: number): string;
  shaped_get_run_direction(shaped: RID, index: number): number;
  shaped_get_run_object(shaped: RID, index: number): any;
  shaped_text_substr(shaped: RID, start: number, length: number): RID;
  shaped_text_get_parent(shaped: RID): RID;
  shaped_text_fit_to_width(shaped: RID, width: number, justification_flags?: number): number;
  shaped_text_tab_align(shaped: RID, tab_stops: PackedFloat32Array): number;
  shaped_text_shape(shaped: RID): boolean;
  shaped_text_is_ready(shaped: RID): boolean;
  shaped_text_has_visible_chars(shaped: RID): boolean;
  shaped_text_get_glyphs(shaped: RID): any[];
  shaped_text_sort_logical(shaped: RID): any[];
  shaped_text_get_glyph_count(shaped: RID): number;
  shaped_text_get_range(shaped: RID): Vector2i;
  shaped_text_get_line_breaks_adv(shaped: RID, width: PackedFloat32Array, start?: number, once?: boolean, break_flags?: number): PackedInt32Array;
  shaped_text_get_line_breaks(shaped: RID, width: number, start?: number, break_flags?: number): PackedInt32Array;
  shaped_text_get_word_breaks(shaped: RID, grapheme_flags?: number, skip_grapheme_flags?: number): PackedInt32Array;
  shaped_text_get_trim_pos(shaped: RID): number;
  shaped_text_get_ellipsis_pos(shaped: RID): number;
  shaped_text_get_ellipsis_glyphs(shaped: RID): any[];
  shaped_text_get_ellipsis_glyph_count(shaped: RID): number;
  shaped_text_overrun_trim_to_width(shaped: RID, width?: number, overrun_trim_flags?: number): void;
  shaped_text_get_objects(shaped: RID): any[];
  shaped_text_get_object_rect(shaped: RID, key: any): Rect2;
  shaped_text_get_object_range(shaped: RID, key: any): Vector2i;
  shaped_text_get_object_glyph(shaped: RID, key: any): number;
  shaped_text_get_size(shaped: RID): Vector2;
  shaped_text_get_ascent(shaped: RID): number;
  shaped_text_get_descent(shaped: RID): number;
  shaped_text_get_width(shaped: RID): number;
  shaped_text_get_underline_position(shaped: RID): number;
  shaped_text_get_underline_thickness(shaped: RID): number;
  shaped_text_get_carets(shaped: RID, position: number): Record<string, any>;
  shaped_text_get_selection(shaped: RID, start: number, end: number): PackedVector2Array;
  shaped_text_hit_test_grapheme(shaped: RID, coords: number): number;
  shaped_text_hit_test_position(shaped: RID, coords: number): number;
  shaped_text_get_grapheme_bounds(shaped: RID, pos: number): Vector2;
  shaped_text_next_grapheme_pos(shaped: RID, pos: number): number;
  shaped_text_prev_grapheme_pos(shaped: RID, pos: number): number;
  shaped_text_get_character_breaks(shaped: RID): PackedInt32Array;
  shaped_text_next_character_pos(shaped: RID, pos: number): number;
  shaped_text_prev_character_pos(shaped: RID, pos: number): number;
  shaped_text_closest_character_pos(shaped: RID, pos: number): number;
  shaped_text_draw(shaped: RID, canvas: RID, pos: Vector2, clip_l?: number, clip_r?: number, color?: Color, oversampling?: number): void;
  shaped_text_draw_outline(shaped: RID, canvas: RID, pos: Vector2, clip_l?: number, clip_r?: number, outline_size?: number, color?: Color, oversampling?: number): void;
  shaped_text_get_dominant_direction_in_range(shaped: RID, start: number, end: number): number;
  format_number(number: string, language?: string): string;
  parse_number(number: string, language?: string): string;
  percent_sign(language?: string): string;
  string_get_word_breaks(string: string, language?: string, chars_per_line?: number): PackedInt32Array;
  string_get_character_breaks(string: string, language?: string): PackedInt32Array;
  is_confusable(string: string, dict: PackedStringArray): number;
  spoof_check(string: string): boolean;
  strip_diacritics(string: string): string;
  is_valid_identifier(string: string): boolean;
  is_valid_letter(unicode: number): boolean;
  string_to_upper(string: string, language?: string): string;
  string_to_lower(string: string, language?: string): string;
  string_to_title(string: string, language?: string): string;
  parse_structured_text(parser_type: number, args: any[], text: string): any[];
}

export declare class TextServerExtension extends TextServer {
}

export declare class Texture extends Resource {
}

export declare class Texture2D extends Texture {
  get_width(): number;
  get_height(): number;
  get_size(): Vector2;
  has_alpha(): boolean;
  draw(canvas_item: RID, position: Vector2, modulate?: Color, transpose?: boolean): void;
  draw_rect(canvas_item: RID, rect: Rect2, tile: boolean, modulate?: Color, transpose?: boolean): void;
  draw_rect_region(canvas_item: RID, rect: Rect2, src_rect: Rect2, modulate?: Color, transpose?: boolean, clip_uv?: boolean): void;
  get_image(): Image;
  create_placeholder(): Resource;
}

export declare class Texture2DArray extends ImageTextureLayered {
  create_placeholder(): Resource;
}

export declare class Texture2DArrayRD extends TextureLayeredRD {
}

export declare class Texture2DRD extends Texture2D {
  texture_rd_rid: RID;
}

export declare class Texture3D extends Texture {
  get_format(): number;
  get_width(): number;
  get_height(): number;
  get_depth(): number;
  has_mipmaps(): boolean;
  get_data(): any[];
  create_placeholder(): Resource;
}

export declare class Texture3DRD extends Texture3D {
  texture_rd_rid: RID;
}

export declare class TextureButton extends BaseButton {
  texture_normal: Texture2D;
  texture_pressed: Texture2D;
  texture_hover: Texture2D;
  texture_disabled: Texture2D;
  texture_focused: Texture2D;
  texture_click_mask: BitMap;
  ignore_texture_size: boolean;
  stretch_mode: number;
  flip_h: boolean;
  flip_v: boolean;
}

export declare class TextureCubemapArrayRD extends TextureLayeredRD {
}

export declare class TextureCubemapRD extends TextureLayeredRD {
}

export declare class TextureLayered extends Texture {
  get_format(): number;
  get_layered_type(): number;
  get_width(): number;
  get_height(): number;
  get_layers(): number;
  has_mipmaps(): boolean;
  get_layer_data(layer: number): Image;
}

export declare class TextureLayeredRD extends TextureLayered {
  texture_rd_rid: RID;
}

export declare class TextureProgressBar extends Range {
  fill_mode: number;
  radial_initial_angle: number;
  radial_fill_degrees: number;
  radial_center_offset: Vector2;
  nine_patch_stretch: boolean;
  texture_under: Texture2D;
  texture_over: Texture2D;
  texture_progress: Texture2D;
  texture_progress_offset: Vector2;
  tint_under: Color;
  tint_over: Color;
  tint_progress: Color;
}

export declare class TextureRect extends Control {
  texture: Texture2D;
  expand_mode: number;
  stretch_mode: number;
  flip_h: boolean;
  flip_v: boolean;
}

export declare class Theme extends Resource {
  default_base_scale: number;
  default_font: Font;
  default_font_size: number;
  set_icon(name: string, theme_type: string, texture: Texture2D): void;
  get_icon(name: string, theme_type: string): Texture2D;
  has_icon(name: string, theme_type: string): boolean;
  rename_icon(old_name: string, name: string, theme_type: string): void;
  clear_icon(name: string, theme_type: string): void;
  get_icon_list(theme_type: string): PackedStringArray;
  get_icon_type_list(): PackedStringArray;
  set_stylebox(name: string, theme_type: string, texture: StyleBox): void;
  get_stylebox(name: string, theme_type: string): StyleBox;
  has_stylebox(name: string, theme_type: string): boolean;
  rename_stylebox(old_name: string, name: string, theme_type: string): void;
  clear_stylebox(name: string, theme_type: string): void;
  get_stylebox_list(theme_type: string): PackedStringArray;
  get_stylebox_type_list(): PackedStringArray;
  set_font(name: string, theme_type: string, font: Font): void;
  get_font(name: string, theme_type: string): Font;
  has_font(name: string, theme_type: string): boolean;
  rename_font(old_name: string, name: string, theme_type: string): void;
  clear_font(name: string, theme_type: string): void;
  get_font_list(theme_type: string): PackedStringArray;
  get_font_type_list(): PackedStringArray;
  set_font_size(name: string, theme_type: string, font_size: number): void;
  get_font_size(name: string, theme_type: string): number;
  has_font_size(name: string, theme_type: string): boolean;
  rename_font_size(old_name: string, name: string, theme_type: string): void;
  clear_font_size(name: string, theme_type: string): void;
  get_font_size_list(theme_type: string): PackedStringArray;
  get_font_size_type_list(): PackedStringArray;
  set_color(name: string, theme_type: string, color: Color): void;
  get_color(name: string, theme_type: string): Color;
  has_color(name: string, theme_type: string): boolean;
  rename_color(old_name: string, name: string, theme_type: string): void;
  clear_color(name: string, theme_type: string): void;
  get_color_list(theme_type: string): PackedStringArray;
  get_color_type_list(): PackedStringArray;
  set_constant(name: string, theme_type: string, constant: number): void;
  get_constant(name: string, theme_type: string): number;
  has_constant(name: string, theme_type: string): boolean;
  rename_constant(old_name: string, name: string, theme_type: string): void;
  clear_constant(name: string, theme_type: string): void;
  get_constant_list(theme_type: string): PackedStringArray;
  get_constant_type_list(): PackedStringArray;
  has_default_base_scale(): boolean;
  has_default_font(): boolean;
  has_default_font_size(): boolean;
  set_theme_item(data_type: number, name: string, theme_type: string, value: any): void;
  get_theme_item(data_type: number, name: string, theme_type: string): any;
  has_theme_item(data_type: number, name: string, theme_type: string): boolean;
  rename_theme_item(data_type: number, old_name: string, name: string, theme_type: string): void;
  clear_theme_item(data_type: number, name: string, theme_type: string): void;
  get_theme_item_list(data_type: number, theme_type: string): PackedStringArray;
  get_theme_item_type_list(data_type: number): PackedStringArray;
  set_type_variation(theme_type: string, base_type: string): void;
  is_type_variation(theme_type: string, base_type: string): boolean;
  clear_type_variation(theme_type: string): void;
  get_type_variation_base(theme_type: string): string;
  get_type_variation_list(base_type: string): PackedStringArray;
  add_type(theme_type: string): void;
  remove_type(theme_type: string): void;
  rename_type(old_theme_type: string, theme_type: string): void;
  get_type_list(): PackedStringArray;
  merge_with(other: Theme): void;
  clear(): void;
}

export declare class TileData extends Object {
  flip_h: boolean;
  flip_v: boolean;
  transpose: boolean;
  texture_origin: Vector2i;
  modulate: Color;
  z_index: number;
  y_sort_origin: number;
  terrain_set: number;
  terrain: number;
  probability: number;
  set_occluder_polygons_count(layer_id: number, polygons_count: number): void;
  get_occluder_polygons_count(layer_id: number): number;
  add_occluder_polygon(layer_id: number): void;
  remove_occluder_polygon(layer_id: number, polygon_index: number): void;
  set_occluder_polygon(layer_id: number, polygon_index: number, polygon: OccluderPolygon2D): void;
  get_occluder_polygon(layer_id: number, polygon_index: number, flip_h?: boolean, flip_v?: boolean, transpose?: boolean): OccluderPolygon2D;
  set_occluder(layer_id: number, occluder_polygon: OccluderPolygon2D): void;
  get_occluder(layer_id: number, flip_h?: boolean, flip_v?: boolean, transpose?: boolean): OccluderPolygon2D;
  set_constant_linear_velocity(layer_id: number, velocity: Vector2): void;
  get_constant_linear_velocity(layer_id: number): Vector2;
  set_constant_angular_velocity(layer_id: number, velocity: number): void;
  get_constant_angular_velocity(layer_id: number): number;
  set_collision_polygons_count(layer_id: number, polygons_count: number): void;
  get_collision_polygons_count(layer_id: number): number;
  add_collision_polygon(layer_id: number): void;
  remove_collision_polygon(layer_id: number, polygon_index: number): void;
  set_collision_polygon_points(layer_id: number, polygon_index: number, polygon: PackedVector2Array): void;
  get_collision_polygon_points(layer_id: number, polygon_index: number): PackedVector2Array;
  set_collision_polygon_one_way(layer_id: number, polygon_index: number, one_way: boolean): void;
  is_collision_polygon_one_way(layer_id: number, polygon_index: number): boolean;
  set_collision_polygon_one_way_margin(layer_id: number, polygon_index: number, one_way_margin: number): void;
  get_collision_polygon_one_way_margin(layer_id: number, polygon_index: number): number;
  set_terrain_peering_bit(peering_bit: number, terrain: number): void;
  get_terrain_peering_bit(peering_bit: number): number;
  is_valid_terrain_peering_bit(peering_bit: number): boolean;
  set_navigation_polygon(layer_id: number, navigation_polygon: NavigationPolygon): void;
  get_navigation_polygon(layer_id: number, flip_h?: boolean, flip_v?: boolean, transpose?: boolean): NavigationPolygon;
  set_custom_data(layer_name: string, value: any): void;
  get_custom_data(layer_name: string): any;
  has_custom_data(layer_name: string): boolean;
  set_custom_data_by_layer_id(layer_id: number, value: any): void;
  get_custom_data_by_layer_id(layer_id: number): any;
}

export declare class TileMap extends Node2D {
  tile_set: TileSet;
  rendering_quadrant_size: number;
  collision_animatable: boolean;
  collision_visibility_mode: number;
  navigation_visibility_mode: number;
  set_navigation_map(layer: number, map: RID): void;
  get_navigation_map(layer: number): RID;
  force_update(layer?: number): void;
  get_layers_count(): number;
  add_layer(to_position: number): void;
  move_layer(layer: number, to_position: number): void;
  remove_layer(layer: number): void;
  set_layer_name(layer: number, name: string): void;
  get_layer_name(layer: number): string;
  set_layer_enabled(layer: number, enabled: boolean): void;
  is_layer_enabled(layer: number): boolean;
  set_layer_modulate(layer: number, modulate: Color): void;
  get_layer_modulate(layer: number): Color;
  set_layer_y_sort_enabled(layer: number, y_sort_enabled: boolean): void;
  is_layer_y_sort_enabled(layer: number): boolean;
  set_layer_y_sort_origin(layer: number, y_sort_origin: number): void;
  get_layer_y_sort_origin(layer: number): number;
  set_layer_z_index(layer: number, z_index: number): void;
  get_layer_z_index(layer: number): number;
  set_layer_navigation_enabled(layer: number, enabled: boolean): void;
  is_layer_navigation_enabled(layer: number): boolean;
  set_layer_navigation_map(layer: number, map: RID): void;
  get_layer_navigation_map(layer: number): RID;
  set_cell(layer: number, coords: Vector2i, source_id?: number, atlas_coords?: Vector2i, alternative_tile?: number): void;
  erase_cell(layer: number, coords: Vector2i): void;
  get_cell_source_id(layer: number, coords: Vector2i, use_proxies?: boolean): number;
  get_cell_atlas_coords(layer: number, coords: Vector2i, use_proxies?: boolean): Vector2i;
  get_cell_alternative_tile(layer: number, coords: Vector2i, use_proxies?: boolean): number;
  get_cell_tile_data(layer: number, coords: Vector2i, use_proxies?: boolean): TileData;
  is_cell_flipped_h(layer: number, coords: Vector2i, use_proxies?: boolean): boolean;
  is_cell_flipped_v(layer: number, coords: Vector2i, use_proxies?: boolean): boolean;
  is_cell_transposed(layer: number, coords: Vector2i, use_proxies?: boolean): boolean;
  get_coords_for_body_rid(body: RID): Vector2i;
  get_layer_for_body_rid(body: RID): number;
  get_pattern(layer: number, coords_array: any[]): TileMapPattern;
  map_pattern(position_in_tilemap: Vector2i, coords_in_pattern: Vector2i, pattern: TileMapPattern): Vector2i;
  set_pattern(layer: number, position: Vector2i, pattern: TileMapPattern): void;
  set_cells_terrain_connect(layer: number, cells: any[], terrain_set: number, terrain: number, ignore_empty_terrains?: boolean): void;
  set_cells_terrain_path(layer: number, path: any[], terrain_set: number, terrain: number, ignore_empty_terrains?: boolean): void;
  fix_invalid_tiles(): void;
  clear_layer(layer: number): void;
  clear(): void;
  update_internals(): void;
  notify_runtime_tile_data_update(layer?: number): void;
  get_surrounding_cells(coords: Vector2i): any[];
  get_used_cells(layer: number): any[];
  get_used_cells_by_id(layer: number, source_id?: number, atlas_coords?: Vector2i, alternative_tile?: number): any[];
  get_used_rect(): Rect2i;
  map_to_local(map_position: Vector2i): Vector2;
  local_to_map(local_position: Vector2): Vector2i;
  get_neighbor_cell(coords: Vector2i, neighbor: number): Vector2i;
}

export declare class TileMapLayer extends Node2D {
  tile_map_data: PackedByteArray;
  enabled: boolean;
  tile_set: TileSet;
  occlusion_enabled: boolean;
  y_sort_origin: number;
  x_draw_order_reversed: boolean;
  rendering_quadrant_size: number;
  collision_enabled: boolean;
  use_kinematic_bodies: boolean;
  collision_visibility_mode: number;
  physics_quadrant_size: number;
  navigation_enabled: boolean;
  navigation_visibility_mode: number;
  set_cell(coords: Vector2i, source_id?: number, atlas_coords?: Vector2i, alternative_tile?: number): void;
  erase_cell(coords: Vector2i): void;
  fix_invalid_tiles(): void;
  clear(): void;
  get_cell_source_id(coords: Vector2i): number;
  get_cell_atlas_coords(coords: Vector2i): Vector2i;
  get_cell_alternative_tile(coords: Vector2i): number;
  get_cell_tile_data(coords: Vector2i): TileData;
  is_cell_flipped_h(coords: Vector2i): boolean;
  is_cell_flipped_v(coords: Vector2i): boolean;
  is_cell_transposed(coords: Vector2i): boolean;
  get_used_cells(): any[];
  get_used_cells_by_id(source_id?: number, atlas_coords?: Vector2i, alternative_tile?: number): any[];
  get_used_rect(): Rect2i;
  get_pattern(coords_array: any[]): TileMapPattern;
  set_pattern(position: Vector2i, pattern: TileMapPattern): void;
  set_cells_terrain_connect(cells: any[], terrain_set: number, terrain: number, ignore_empty_terrains?: boolean): void;
  set_cells_terrain_path(path: any[], terrain_set: number, terrain: number, ignore_empty_terrains?: boolean): void;
  has_body_rid(body: RID): boolean;
  get_coords_for_body_rid(body: RID): Vector2i;
  update_internals(): void;
  notify_runtime_tile_data_update(): void;
  map_pattern(position_in_tilemap: Vector2i, coords_in_pattern: Vector2i, pattern: TileMapPattern): Vector2i;
  get_surrounding_cells(coords: Vector2i): any[];
  get_neighbor_cell(coords: Vector2i, neighbor: number): Vector2i;
  map_to_local(map_position: Vector2i): Vector2;
  local_to_map(local_position: Vector2): Vector2i;
  set_navigation_map(map: RID): void;
  get_navigation_map(): RID;
}

export declare class TileMapPattern extends Resource {
  set_cell(coords: Vector2i, source_id?: number, atlas_coords?: Vector2i, alternative_tile?: number): void;
  has_cell(coords: Vector2i): boolean;
  remove_cell(coords: Vector2i, update_size: boolean): void;
  get_cell_source_id(coords: Vector2i): number;
  get_cell_atlas_coords(coords: Vector2i): Vector2i;
  get_cell_alternative_tile(coords: Vector2i): number;
  get_used_cells(): any[];
  get_size(): Vector2i;
  set_size(size: Vector2i): void;
  is_empty(): boolean;
}

export declare class TileSet extends Resource {
  tile_shape: number;
  tile_layout: number;
  tile_offset_axis: number;
  tile_size: Vector2i;
  uv_clipping: boolean;
  get_next_source_id(): number;
  add_source(source: TileSetSource, atlas_source_id_override?: number): number;
  remove_source(source_id: number): void;
  set_source_id(source_id: number, new_source_id: number): void;
  get_source_count(): number;
  get_source_id(index: number): number;
  has_source(source_id: number): boolean;
  get_source(source_id: number): TileSetSource;
  get_occlusion_layers_count(): number;
  add_occlusion_layer(to_position?: number): void;
  move_occlusion_layer(layer_index: number, to_position: number): void;
  remove_occlusion_layer(layer_index: number): void;
  set_occlusion_layer_light_mask(layer_index: number, light_mask: number): void;
  get_occlusion_layer_light_mask(layer_index: number): number;
  set_occlusion_layer_sdf_collision(layer_index: number, sdf_collision: boolean): void;
  get_occlusion_layer_sdf_collision(layer_index: number): boolean;
  get_physics_layers_count(): number;
  add_physics_layer(to_position?: number): void;
  move_physics_layer(layer_index: number, to_position: number): void;
  remove_physics_layer(layer_index: number): void;
  set_physics_layer_collision_layer(layer_index: number, layer: number): void;
  get_physics_layer_collision_layer(layer_index: number): number;
  set_physics_layer_collision_mask(layer_index: number, mask: number): void;
  get_physics_layer_collision_mask(layer_index: number): number;
  set_physics_layer_collision_priority(layer_index: number, priority: number): void;
  get_physics_layer_collision_priority(layer_index: number): number;
  set_physics_layer_physics_material(layer_index: number, physics_material: PhysicsMaterial): void;
  get_physics_layer_physics_material(layer_index: number): PhysicsMaterial;
  get_terrain_sets_count(): number;
  add_terrain_set(to_position?: number): void;
  move_terrain_set(terrain_set: number, to_position: number): void;
  remove_terrain_set(terrain_set: number): void;
  set_terrain_set_mode(terrain_set: number, mode: number): void;
  get_terrain_set_mode(terrain_set: number): number;
  get_terrains_count(terrain_set: number): number;
  add_terrain(terrain_set: number, to_position?: number): void;
  move_terrain(terrain_set: number, terrain_index: number, to_position: number): void;
  remove_terrain(terrain_set: number, terrain_index: number): void;
  set_terrain_name(terrain_set: number, terrain_index: number, name: string): void;
  get_terrain_name(terrain_set: number, terrain_index: number): string;
  set_terrain_color(terrain_set: number, terrain_index: number, color: Color): void;
  get_terrain_color(terrain_set: number, terrain_index: number): Color;
  get_navigation_layers_count(): number;
  add_navigation_layer(to_position?: number): void;
  move_navigation_layer(layer_index: number, to_position: number): void;
  remove_navigation_layer(layer_index: number): void;
  set_navigation_layer_layers(layer_index: number, layers: number): void;
  get_navigation_layer_layers(layer_index: number): number;
  set_navigation_layer_layer_value(layer_index: number, layer_number: number, value: boolean): void;
  get_navigation_layer_layer_value(layer_index: number, layer_number: number): boolean;
  get_custom_data_layers_count(): number;
  add_custom_data_layer(to_position?: number): void;
  move_custom_data_layer(layer_index: number, to_position: number): void;
  remove_custom_data_layer(layer_index: number): void;
  get_custom_data_layer_by_name(layer_name: string): number;
  set_custom_data_layer_name(layer_index: number, layer_name: string): void;
  has_custom_data_layer_by_name(layer_name: string): boolean;
  get_custom_data_layer_name(layer_index: number): string;
  set_custom_data_layer_type(layer_index: number, layer_type: number): void;
  get_custom_data_layer_type(layer_index: number): number;
  set_source_level_tile_proxy(source_from: number, source_to: number): void;
  get_source_level_tile_proxy(source_from: number): number;
  has_source_level_tile_proxy(source_from: number): boolean;
  remove_source_level_tile_proxy(source_from: number): void;
  set_coords_level_tile_proxy(p_source_from: number, coords_from: Vector2i, source_to: number, coords_to: Vector2i): void;
  get_coords_level_tile_proxy(source_from: number, coords_from: Vector2i): any[];
  has_coords_level_tile_proxy(source_from: number, coords_from: Vector2i): boolean;
  remove_coords_level_tile_proxy(source_from: number, coords_from: Vector2i): void;
  set_alternative_level_tile_proxy(source_from: number, coords_from: Vector2i, alternative_from: number, source_to: number, coords_to: Vector2i, alternative_to: number): void;
  get_alternative_level_tile_proxy(source_from: number, coords_from: Vector2i, alternative_from: number): any[];
  has_alternative_level_tile_proxy(source_from: number, coords_from: Vector2i, alternative_from: number): boolean;
  remove_alternative_level_tile_proxy(source_from: number, coords_from: Vector2i, alternative_from: number): void;
  map_tile_proxy(source_from: number, coords_from: Vector2i, alternative_from: number): any[];
  cleanup_invalid_tile_proxies(): void;
  clear_tile_proxies(): void;
  add_pattern(pattern: TileMapPattern, index?: number): number;
  get_pattern(index?: number): TileMapPattern;
  remove_pattern(index: number): void;
  get_patterns_count(): number;
}

export declare class TileSetAtlasSource extends TileSetSource {
  texture: Texture2D;
  margins: Vector2i;
  separation: Vector2i;
  texture_region_size: Vector2i;
  use_texture_padding: boolean;
  create_tile(atlas_coords: Vector2i, size?: Vector2i): void;
  remove_tile(atlas_coords: Vector2i): void;
  move_tile_in_atlas(atlas_coords: Vector2i, new_atlas_coords?: Vector2i, new_size?: Vector2i): void;
  get_tile_size_in_atlas(atlas_coords: Vector2i): Vector2i;
  has_room_for_tile(atlas_coords: Vector2i, size: Vector2i, animation_columns: number, animation_separation: Vector2i, frames_count: number, ignored_tile?: Vector2i): boolean;
  get_tiles_to_be_removed_on_change(texture: Texture2D, margins: Vector2i, separation: Vector2i, texture_region_size: Vector2i): PackedVector2Array;
  get_tile_at_coords(atlas_coords: Vector2i): Vector2i;
  has_tiles_outside_texture(): boolean;
  clear_tiles_outside_texture(): void;
  set_tile_animation_columns(atlas_coords: Vector2i, frame_columns: number): void;
  get_tile_animation_columns(atlas_coords: Vector2i): number;
  set_tile_animation_separation(atlas_coords: Vector2i, separation: Vector2i): void;
  get_tile_animation_separation(atlas_coords: Vector2i): Vector2i;
  set_tile_animation_speed(atlas_coords: Vector2i, speed: number): void;
  get_tile_animation_speed(atlas_coords: Vector2i): number;
  set_tile_animation_mode(atlas_coords: Vector2i, mode: number): void;
  get_tile_animation_mode(atlas_coords: Vector2i): number;
  set_tile_animation_frames_count(atlas_coords: Vector2i, frames_count: number): void;
  get_tile_animation_frames_count(atlas_coords: Vector2i): number;
  set_tile_animation_frame_duration(atlas_coords: Vector2i, frame_index: number, duration: number): void;
  get_tile_animation_frame_duration(atlas_coords: Vector2i, frame_index: number): number;
  get_tile_animation_total_duration(atlas_coords: Vector2i): number;
  create_alternative_tile(atlas_coords: Vector2i, alternative_id_override?: number): number;
  remove_alternative_tile(atlas_coords: Vector2i, alternative_tile: number): void;
  set_alternative_tile_id(atlas_coords: Vector2i, alternative_tile: number, new_id: number): void;
  get_next_alternative_tile_id(atlas_coords: Vector2i): number;
  get_tile_data(atlas_coords: Vector2i, alternative_tile: number): TileData;
  get_atlas_grid_size(): Vector2i;
  get_tile_texture_region(atlas_coords: Vector2i, frame?: number): Rect2i;
  get_runtime_texture(): Texture2D;
  get_runtime_tile_texture_region(atlas_coords: Vector2i, frame: number): Rect2i;
}

export declare class TileSetScenesCollectionSource extends TileSetSource {
  get_scene_tiles_count(): number;
  get_scene_tile_id(index: number): number;
  has_scene_tile_id(id: number): boolean;
  create_scene_tile(packed_scene: PackedScene, id_override?: number): number;
  set_scene_tile_id(id: number, new_id: number): void;
  set_scene_tile_scene(id: number, packed_scene: PackedScene): void;
  get_scene_tile_scene(id: number): PackedScene;
  set_scene_tile_display_placeholder(id: number, display_placeholder: boolean): void;
  get_scene_tile_display_placeholder(id: number): boolean;
  remove_scene_tile(id: number): void;
  get_next_scene_tile_id(): number;
}

export declare class TileSetSource extends Resource {
  get_tiles_count(): number;
  get_tile_id(index: number): Vector2i;
  has_tile(atlas_coords: Vector2i): boolean;
  get_alternative_tiles_count(atlas_coords: Vector2i): number;
  get_alternative_tile_id(atlas_coords: Vector2i, index: number): number;
  has_alternative_tile(atlas_coords: Vector2i, alternative_tile: number): boolean;
}

export declare class Timer extends Node {
  process_callback: number;
  wait_time: number;
  one_shot: boolean;
  autostart: boolean;
  paused: boolean;
  ignore_time_scale: boolean;
  readonly time_left: number;
  start(time_sec?: number): void;
  stop(): void;
  is_stopped(): boolean;
}

export declare class TorusMesh extends PrimitiveMesh {
  inner_radius: number;
  outer_radius: number;
  rings: number;
  ring_segments: number;
}

export declare class TouchScreenButton extends Node2D {
  texture_normal: Texture2D;
  texture_pressed: Texture2D;
  bitmask: BitMap;
  shape: Shape2D;
  shape_centered: boolean;
  shape_visible: boolean;
  passby_press: boolean;
  visibility_mode: number;
  is_pressed(): boolean;
}

export declare class Translation extends Resource {
  locale: string;
  add_message(src_message: string, xlated_message: string, context?: string): void;
  add_plural_message(src_message: string, xlated_messages: PackedStringArray, context?: string): void;
  get_message(src_message: string, context?: string): string;
  get_plural_message(src_message: string, src_plural_message: string, n: number, context?: string): string;
  erase_message(src_message: string, context?: string): void;
  get_message_list(): PackedStringArray;
  get_translated_message_list(): PackedStringArray;
  get_message_count(): number;
}

export declare class TranslationDomain extends RefCounted {
  enabled: boolean;
  pseudolocalization_enabled: boolean;
  pseudolocalization_accents_enabled: boolean;
  pseudolocalization_double_vowels_enabled: boolean;
  pseudolocalization_fake_bidi_enabled: boolean;
  pseudolocalization_override_enabled: boolean;
  pseudolocalization_skip_placeholders_enabled: boolean;
  pseudolocalization_expansion_ratio: number;
  pseudolocalization_prefix: string;
  pseudolocalization_suffix: string;
  get_translation_object(locale: string): Translation;
  add_translation(translation: Translation): void;
  remove_translation(translation: Translation): void;
  clear(): void;
  translate(message: string, context?: string): string;
  translate_plural(message: string, message_plural: string, n: number, context?: string): string;
  get_locale_override(): string;
  set_locale_override(locale: string): void;
  pseudolocalize(message: string): string;
}

export declare class Tree extends Control {
  columns: number;
  column_titles_visible: boolean;
  allow_reselect: boolean;
  allow_rmb_select: boolean;
  allow_search: boolean;
  hide_folding: boolean;
  enable_recursive_folding: boolean;
  hide_root: boolean;
  drop_mode_flags: number;
  select_mode: number;
  scroll_horizontal_enabled: boolean;
  scroll_vertical_enabled: boolean;
  auto_tooltip: boolean;
  clear(): void;
  create_item(parent?: TreeItem, index?: number): TreeItem;
  get_root(): TreeItem;
  set_column_custom_minimum_width(column: number, min_width: number): void;
  set_column_expand(column: number, expand: boolean): void;
  set_column_expand_ratio(column: number, ratio: number): void;
  set_column_clip_content(column: number, enable: boolean): void;
  is_column_expanding(column: number): boolean;
  is_column_clipping_content(column: number): boolean;
  get_column_expand_ratio(column: number): number;
  get_column_width(column: number): number;
  get_next_selected(from: TreeItem): TreeItem;
  get_selected(): TreeItem;
  set_selected(item: TreeItem, column: number): void;
  get_selected_column(): number;
  get_pressed_button(): number;
  deselect_all(): void;
  get_edited(): TreeItem;
  get_edited_column(): number;
  edit_selected(force_edit?: boolean): boolean;
  get_custom_popup_rect(): Rect2;
  get_item_area_rect(item: TreeItem, column?: number, button_index?: number): Rect2;
  get_item_at_position(position: Vector2): TreeItem;
  get_column_at_position(position: Vector2): number;
  get_drop_section_at_position(position: Vector2): number;
  get_button_id_at_position(position: Vector2): number;
  ensure_cursor_is_visible(): void;
  set_column_title(column: number, title: string): void;
  get_column_title(column: number): string;
  set_column_title_alignment(column: number, title_alignment: number): void;
  get_column_title_alignment(column: number): number;
  set_column_title_direction(column: number, direction: number): void;
  get_column_title_direction(column: number): number;
  set_column_title_language(column: number, language: string): void;
  get_column_title_language(column: number): string;
  get_scroll(): Vector2;
  scroll_to_item(item: TreeItem, center_on_item?: boolean): void;
}

export declare class TreeItem extends Object {
  collapsed: boolean;
  visible: boolean;
  disable_folding: boolean;
  custom_minimum_height: number;
  set_cell_mode(column: number, mode: number): void;
  get_cell_mode(column: number): number;
  set_auto_translate_mode(column: number, mode: number): void;
  get_auto_translate_mode(column: number): number;
  set_edit_multiline(column: number, multiline: boolean): void;
  is_edit_multiline(column: number): boolean;
  set_checked(column: number, checked: boolean): void;
  set_indeterminate(column: number, indeterminate: boolean): void;
  is_checked(column: number): boolean;
  is_indeterminate(column: number): boolean;
  propagate_check(column: number, emit_signal?: boolean): void;
  set_text(column: number, text: string): void;
  get_text(column: number): string;
  set_description(column: number, description: string): void;
  get_description(column: number): string;
  set_text_direction(column: number, direction: number): void;
  get_text_direction(column: number): number;
  set_autowrap_mode(column: number, autowrap_mode: number): void;
  get_autowrap_mode(column: number): number;
  set_text_overrun_behavior(column: number, overrun_behavior: number): void;
  get_text_overrun_behavior(column: number): number;
  set_structured_text_bidi_override(column: number, parser: number): void;
  get_structured_text_bidi_override(column: number): number;
  set_structured_text_bidi_override_options(column: number, args: any[]): void;
  get_structured_text_bidi_override_options(column: number): any[];
  set_language(column: number, language: string): void;
  get_language(column: number): string;
  set_suffix(column: number, text: string): void;
  get_suffix(column: number): string;
  set_icon(column: number, texture: Texture2D): void;
  get_icon(column: number): Texture2D;
  set_icon_overlay(column: number, texture: Texture2D): void;
  get_icon_overlay(column: number): Texture2D;
  set_icon_region(column: number, region: Rect2): void;
  get_icon_region(column: number): Rect2;
  set_icon_max_width(column: number, width: number): void;
  get_icon_max_width(column: number): number;
  set_icon_modulate(column: number, modulate: Color): void;
  get_icon_modulate(column: number): Color;
  set_range(column: number, value: number): void;
  get_range(column: number): number;
  set_range_config(column: number, min: number, max: number, step: number, expr?: boolean): void;
  get_range_config(column: number): Record<string, any>;
  set_metadata(column: number, meta: any): void;
  get_metadata(column: number): any;
  set_custom_draw(column: number, object: any, callback: string): void;
  set_custom_draw_callback(column: number, callback: Callable): void;
  get_custom_draw_callback(column: number): Callable;
  set_collapsed_recursive(enable: boolean): void;
  is_any_collapsed(only_visible?: boolean): boolean;
  is_visible_in_tree(): boolean;
  uncollapse_tree(): void;
  set_selectable(column: number, selectable: boolean): void;
  is_selectable(column: number): boolean;
  is_selected(column: number): boolean;
  select(column: number): void;
  deselect(column: number): void;
  set_editable(column: number, enabled: boolean): void;
  is_editable(column: number): boolean;
  set_custom_color(column: number, color: Color): void;
  get_custom_color(column: number): Color;
  clear_custom_color(column: number): void;
  set_custom_font(column: number, font: Font): void;
  get_custom_font(column: number): Font;
  set_custom_font_size(column: number, font_size: number): void;
  get_custom_font_size(column: number): number;
  set_custom_bg_color(column: number, color: Color, just_outline?: boolean): void;
  clear_custom_bg_color(column: number): void;
  get_custom_bg_color(column: number): Color;
  set_custom_as_button(column: number, enable: boolean): void;
  is_custom_set_as_button(column: number): boolean;
  clear_buttons(): void;
  add_button(column: number, button: Texture2D, id?: number, disabled?: boolean, tooltip_text?: string, description?: string): void;
  get_button_count(column: number): number;
  get_button_tooltip_text(column: number, button_index: number): string;
  get_button_id(column: number, button_index: number): number;
  get_button_by_id(column: number, id: number): number;
  get_button_color(column: number, id: number): Color;
  get_button(column: number, button_index: number): Texture2D;
  set_button_tooltip_text(column: number, button_index: number, tooltip: string): void;
  set_button(column: number, button_index: number, button: Texture2D): void;
  erase_button(column: number, button_index: number): void;
  set_button_description(column: number, button_index: number, description: string): void;
  set_button_disabled(column: number, button_index: number, disabled: boolean): void;
  set_button_color(column: number, button_index: number, color: Color): void;
  is_button_disabled(column: number, button_index: number): boolean;
  set_tooltip_text(column: number, tooltip: string): void;
  get_tooltip_text(column: number): string;
  set_text_alignment(column: number, text_alignment: number): void;
  get_text_alignment(column: number): number;
  set_expand_right(column: number, enable: boolean): void;
  get_expand_right(column: number): boolean;
  create_child(index?: number): TreeItem;
  add_child(child: TreeItem): void;
  remove_child(child: TreeItem): void;
  get_tree(): Tree;
  get_next(): TreeItem;
  get_prev(): TreeItem;
  get_parent(): TreeItem;
  get_first_child(): TreeItem;
  get_next_in_tree(wrap?: boolean): TreeItem;
  get_prev_in_tree(wrap?: boolean): TreeItem;
  get_next_visible(wrap?: boolean): TreeItem;
  get_prev_visible(wrap?: boolean): TreeItem;
  get_child(index: number): TreeItem;
  get_child_count(): number;
  get_children(): any[];
  get_index(): number;
  move_before(item: TreeItem): void;
  move_after(item: TreeItem): void;
}

export declare class TriangleMesh extends RefCounted {
  create_from_faces(faces: PackedVector3Array): boolean;
  get_faces(): PackedVector3Array;
  intersect_segment(begin: Vector3, end: Vector3): Record<string, any>;
  intersect_ray(begin: Vector3, dir: Vector3): Record<string, any>;
}

export declare class TubeTrailMesh extends PrimitiveMesh {
  radius: number;
  radial_steps: number;
  sections: number;
  section_length: number;
  section_rings: number;
  cap_top: boolean;
  cap_bottom: boolean;
  curve: Curve;
}

export declare class Tween extends RefCounted {
  tween_property(object: any, property: string, final_val: any, duration: number): PropertyTweener;
  tween_interval(time: number): IntervalTweener;
  tween_callback(callback: Callable): CallbackTweener;
  tween_method(method: Callable, from: any, to: any, duration: number): MethodTweener;
  tween_subtween(subtween: Tween): SubtweenTweener;
  custom_step(delta: number): boolean;
  stop(): void;
  pause(): void;
  play(): void;
  kill(): void;
  get_total_elapsed_time(): number;
  is_running(): boolean;
  is_valid(): boolean;
  bind_node(node: Node): Tween;
  set_process_mode(mode: number): Tween;
  set_pause_mode(mode: number): Tween;
  set_ignore_time_scale(ignore?: boolean): Tween;
  set_parallel(parallel?: boolean): Tween;
  set_loops(loops?: number): Tween;
  get_loops_left(): number;
  set_speed_scale(speed: number): Tween;
  set_trans(trans: number): Tween;
  set_ease(ease: number): Tween;
  parallel(): Tween;
  chain(): Tween;
  interpolate_value(initial_value: any, delta_value: any, elapsed_time: number, duration: number, trans_type: number, ease_type: number): any;
}

export declare class Tweener extends RefCounted {
}

export declare class UPNP extends RefCounted {
  discover_multicast_if: string;
  discover_local_port: number;
  discover_ipv6: boolean;
  get_device_count(): number;
  get_device(index: number): UPNPDevice;
  add_device(device: UPNPDevice): void;
  set_device(index: number, device: UPNPDevice): void;
  remove_device(index: number): void;
  clear_devices(): void;
  get_gateway(): UPNPDevice;
  discover(timeout?: number, ttl?: number, device_filter?: string): number;
  query_external_address(): string;
  add_port_mapping(port: number, port_internal?: number, desc?: string, proto?: string, duration?: number): number;
  delete_port_mapping(port: number, proto?: string): number;
}

export declare class UPNPDevice extends RefCounted {
  description_url: string;
  service_type: string;
  igd_control_url: string;
  igd_service_type: string;
  igd_our_addr: string;
  igd_status: number;
  is_valid_gateway(): boolean;
  query_external_address(): string;
  add_port_mapping(port: number, port_internal?: number, desc?: string, proto?: string, duration?: number): number;
  delete_port_mapping(port: number, proto?: string): number;
}

export declare class UndoRedo extends Object {
  max_steps: number;
  create_action(name: string, merge_mode?: number, backward_undo_ops?: boolean): void;
  commit_action(execute?: boolean): void;
  is_committing_action(): boolean;
  add_do_method(callable: Callable): void;
  add_undo_method(callable: Callable): void;
  add_do_property(object: any, property: string, value: any): void;
  add_undo_property(object: any, property: string, value: any): void;
  add_do_reference(object: any): void;
  add_undo_reference(object: any): void;
  start_force_keep_in_merge_ends(): void;
  end_force_keep_in_merge_ends(): void;
  get_history_count(): number;
  get_current_action(): number;
  get_action_name(id: number): string;
  clear_history(increase_version?: boolean): void;
  get_current_action_name(): string;
  has_undo(): boolean;
  has_redo(): boolean;
  get_version(): number;
  redo(): boolean;
  undo(): boolean;
}

export declare class UniformSetCacheRD extends Object {
  get_cache(shader: RID, set: number, uniforms: any[]): RID;
}

export declare class VBoxContainer extends BoxContainer {
}

export declare class VFlowContainer extends FlowContainer {
}

export declare class VScrollBar extends ScrollBar {
}

export declare class VSeparator extends Separator {
}

export declare class VSlider extends Slider {
}

export declare class VSplitContainer extends SplitContainer {
}

export declare class VehicleBody3D extends RigidBody3D {
  engine_force: number;
  brake: number;
  steering: number;
}

export declare class VehicleWheel3D extends Node3D {
  engine_force: number;
  brake: number;
  steering: number;
  use_as_traction: boolean;
  use_as_steering: boolean;
  wheel_roll_influence: number;
  wheel_radius: number;
  wheel_rest_length: number;
  wheel_friction_slip: number;
  suspension_travel: number;
  suspension_stiffness: number;
  suspension_max_force: number;
  damping_compression: number;
  damping_relaxation: number;
  is_in_contact(): boolean;
  get_contact_body(): Node3D;
  get_contact_point(): Vector3;
  get_contact_normal(): Vector3;
  get_skidinfo(): number;
  get_rpm(): number;
}

export declare class VideoStream extends Resource {
  file: string;
}

export declare class VideoStreamPlayback extends Resource {
  mix_audio(num_frames: number, buffer?: PackedFloat32Array, offset?: number): number;
}

export declare class VideoStreamPlayer extends Control {
  audio_track: number;
  stream: VideoStream;
  volume_db: number;
  volume: number;
  speed_scale: number;
  autoplay: boolean;
  paused: boolean;
  expand: boolean;
  loop: boolean;
  buffering_msec: number;
  stream_position: number;
  bus: string;
  play(): void;
  stop(): void;
  is_playing(): boolean;
  get_stream_name(): string;
  get_stream_length(): number;
  get_video_texture(): Texture2D;
}

export declare class VideoStreamTheora extends VideoStream {
}

export declare class Viewport extends Node {
  disable_3d: boolean;
  use_xr: boolean;
  own_world_3d: boolean;
  world_3d: World3D;
  world_2d: World2D;
  transparent_bg: boolean;
  handle_input_locally: boolean;
  snap_2d_transforms_to_pixel: boolean;
  snap_2d_vertices_to_pixel: boolean;
  msaa_2d: number;
  msaa_3d: number;
  screen_space_aa: number;
  use_taa: boolean;
  use_debanding: boolean;
  use_occlusion_culling: boolean;
  mesh_lod_threshold: number;
  debug_draw: number;
  use_hdr_2d: boolean;
  scaling_3d_mode: number;
  texture_mipmap_bias: number;
  anisotropic_filtering_level: number;
  fsr_sharpness: number;
  vrs_mode: number;
  vrs_update_mode: number;
  vrs_texture: Texture2D;
  canvas_item_default_texture_filter: number;
  canvas_item_default_texture_repeat: number;
  audio_listener_enable_2d: boolean;
  audio_listener_enable_3d: boolean;
  physics_object_picking: boolean;
  physics_object_picking_sort: boolean;
  physics_object_picking_first_only: boolean;
  gui_disable_input: boolean;
  gui_snap_controls_to_pixels: boolean;
  gui_embed_subwindows: boolean;
  sdf_oversize: number;
  sdf_scale: number;
  positional_shadow_atlas_size: number;
  positional_shadow_atlas_16_bits: boolean;
  canvas_transform: Transform2D;
  global_canvas_transform: Transform2D;
  canvas_cull_mask: number;
  oversampling: boolean;
  oversampling_override: number;
  find_world_2d(): World2D;
  get_stretch_transform(): Transform2D;
  get_final_transform(): Transform2D;
  get_screen_transform(): Transform2D;
  get_visible_rect(): Rect2;
  get_render_info(type: number, info: number): number;
  get_texture(): ViewportTexture;
  get_viewport_rid(): RID;
  push_text_input(text: string): void;
  push_input(event: InputEvent, in_local_coords?: boolean): void;
  push_unhandled_input(event: InputEvent, in_local_coords?: boolean): void;
  notify_mouse_entered(): void;
  notify_mouse_exited(): void;
  get_mouse_position(): Vector2;
  warp_mouse(position: Vector2): void;
  update_mouse_cursor_state(): void;
  gui_cancel_drag(): void;
  gui_get_drag_data(): any;
  gui_get_drag_description(): string;
  gui_set_drag_description(description: string): void;
  gui_is_dragging(): boolean;
  gui_is_drag_successful(): boolean;
  gui_release_focus(): void;
  gui_get_focus_owner(): Control;
  gui_get_hovered_control(): Control;
  set_input_as_handled(): void;
  is_input_handled(): boolean;
  get_embedded_subwindows(): any[];
  set_canvas_cull_mask_bit(layer: number, enable: boolean): void;
  get_canvas_cull_mask_bit(layer: number): boolean;
  get_audio_listener_2d(): AudioListener2D;
  get_camera_2d(): Camera2D;
  find_world_3d(): World3D;
  get_audio_listener_3d(): AudioListener3D;
  get_camera_3d(): Camera3D;
}

export declare class ViewportTexture extends Texture2D {
  viewport_path: string;
}

export declare class VisibleOnScreenEnabler2D extends VisibleOnScreenNotifier2D {
  enable_mode: number;
  enable_node_path: string;
}

export declare class VisibleOnScreenEnabler3D extends VisibleOnScreenNotifier3D {
  enable_mode: number;
  enable_node_path: string;
}

export declare class VisibleOnScreenNotifier2D extends Node2D {
  rect: Rect2;
  show_rect: boolean;
  is_on_screen(): boolean;
}

export declare class VisibleOnScreenNotifier3D extends VisualInstance3D {
  aabb: AABB;
  is_on_screen(): boolean;
}

export declare class VisualInstance3D extends Node3D {
  layers: number;
  sorting_offset: number;
  sorting_use_aabb_center: boolean;
  set_base(base: RID): void;
  get_base(): RID;
  get_instance(): RID;
  set_layer_mask_value(layer_number: number, value: boolean): void;
  get_layer_mask_value(layer_number: number): boolean;
  get_aabb(): AABB;
}

export declare class VoxelGI extends VisualInstance3D {
  subdiv: number;
  size: Vector3;
  data: VoxelGIData;
  bake(from_node?: Node, create_visual_debug?: boolean): void;
  debug_bake(): void;
}

export declare class VoxelGIData extends Resource {
  dynamic_range: number;
  energy: number;
  bias: number;
  normal_bias: number;
  propagation: number;
  use_two_bounces: boolean;
  interior: boolean;
  allocate(to_cell_xform: Transform3D, aabb: AABB, octree_size: Vector3, octree_cells: PackedByteArray, data_cells: PackedByteArray, distance_field: PackedByteArray, level_counts: PackedInt32Array): void;
  get_bounds(): AABB;
  get_octree_size(): Vector3;
  get_to_cell_xform(): Transform3D;
  get_octree_cells(): PackedByteArray;
  get_data_cells(): PackedByteArray;
  get_level_counts(): PackedInt32Array;
}

export declare class WeakRef extends RefCounted {
  get_ref(): any;
}

export declare class WebRTCDataChannel extends PacketPeer {
  write_mode: number;
  poll(): number;
  close(): void;
  was_string_packet(): boolean;
  get_ready_state(): number;
  get_label(): string;
  is_ordered(): boolean;
  get_id(): number;
  get_max_packet_life_time(): number;
  get_max_retransmits(): number;
  get_protocol(): string;
  is_negotiated(): boolean;
  get_buffered_amount(): number;
}

export declare class WebRTCDataChannelExtension extends WebRTCDataChannel {
}

export declare class WebRTCMultiplayerPeer extends MultiplayerPeer {
  create_server(channels_config?: any[]): number;
  create_client(peer_id: number, channels_config?: any[]): number;
  create_mesh(peer_id: number, channels_config?: any[]): number;
  add_peer(peer: WebRTCPeerConnection, peer_id: number, unreliable_lifetime?: number): number;
  remove_peer(peer_id: number): void;
  has_peer(peer_id: number): boolean;
  get_peer(peer_id: number): Record<string, any>;
  get_peers(): Record<string, any>;
}

export declare class WebRTCPeerConnection extends RefCounted {
  set_default_extension(extension_class: string): void;
  initialize(configuration?: Record<string, any>): number;
  create_data_channel(label: string, options?: Record<string, any>): WebRTCDataChannel;
  create_offer(): number;
  set_local_description(type: string, sdp: string): number;
  set_remote_description(type: string, sdp: string): number;
  add_ice_candidate(media: string, index: number, name: string): number;
  poll(): number;
  close(): void;
  get_connection_state(): number;
  get_gathering_state(): number;
  get_signaling_state(): number;
}

export declare class WebRTCPeerConnectionExtension extends WebRTCPeerConnection {
}

export declare class WebSocketMultiplayerPeer extends MultiplayerPeer {
  supported_protocols: PackedStringArray;
  handshake_headers: PackedStringArray;
  inbound_buffer_size: number;
  outbound_buffer_size: number;
  handshake_timeout: number;
  max_queued_packets: number;
  create_client(url: string, tls_client_options?: TLSOptions): number;
  create_server(port: number, bind_address?: string, tls_server_options?: TLSOptions): number;
  get_peer(peer_id: number): WebSocketPeer;
  get_peer_address(id: number): string;
  get_peer_port(id: number): number;
}

export declare class WebXRInterface extends XRInterface {
  session_mode: string;
  required_features: string;
  optional_features: string;
  requested_reference_space_types: string;
  readonly reference_space_type: string;
  readonly enabled_features: string;
  readonly visibility_state: string;
  is_session_supported(session_mode: string): void;
  is_input_source_active(input_source_id: number): boolean;
  get_input_source_tracker(input_source_id: number): XRControllerTracker;
  get_input_source_target_ray_mode(input_source_id: number): number;
  get_display_refresh_rate(): number;
  set_display_refresh_rate(refresh_rate: number): void;
  get_available_display_refresh_rates(): any[];
}

export declare class Window extends Viewport {
  mode: number;
  title: string;
  initial_position: number;
  position: Vector2i;
  size: Vector2i;
  current_screen: number;
  mouse_passthrough_polygon: PackedVector2Array;
  visible: boolean;
  wrap_controls: boolean;
  transient: boolean;
  transient_to_focused: boolean;
  exclusive: boolean;
  force_native: boolean;
  min_size: Vector2i;
  max_size: Vector2i;
  keep_title_visible: boolean;
  content_scale_size: Vector2i;
  content_scale_mode: number;
  content_scale_aspect: number;
  content_scale_stretch: number;
  content_scale_factor: number;
  auto_translate: boolean;
  accessibility_name: string;
  accessibility_description: string;
  theme: Theme;
  move_to_center(): void;
  reset_size(): void;
  get_position_with_decorations(): Vector2i;
  get_size_with_decorations(): Vector2i;
  is_maximize_allowed(): boolean;
  request_attention(): void;
  move_to_foreground(): void;
  hide(): void;
  show(): void;
  set_unparent_when_invisible(unparent: boolean): void;
  can_draw(): boolean;
  has_focus(): boolean;
  grab_focus(): void;
  start_drag(): void;
  start_resize(edge: number): void;
  set_ime_active(active: boolean): void;
  set_ime_position(position: Vector2i): void;
  is_embedded(): boolean;
  get_contents_minimum_size(): Vector2;
  child_controls_changed(): void;
  begin_bulk_theme_override(): void;
  end_bulk_theme_override(): void;
  add_theme_icon_override(name: string, texture: Texture2D): void;
  add_theme_stylebox_override(name: string, stylebox: StyleBox): void;
  add_theme_font_override(name: string, font: Font): void;
  add_theme_font_size_override(name: string, font_size: number): void;
  add_theme_color_override(name: string, color: Color): void;
  add_theme_constant_override(name: string, constant: number): void;
  remove_theme_icon_override(name: string): void;
  remove_theme_stylebox_override(name: string): void;
  remove_theme_font_override(name: string): void;
  remove_theme_font_size_override(name: string): void;
  remove_theme_color_override(name: string): void;
  remove_theme_constant_override(name: string): void;
  get_theme_icon(name: string, theme_type?: string): Texture2D;
  get_theme_stylebox(name: string, theme_type?: string): StyleBox;
  get_theme_font(name: string, theme_type?: string): Font;
  get_theme_font_size(name: string, theme_type?: string): number;
  get_theme_color(name: string, theme_type?: string): Color;
  get_theme_constant(name: string, theme_type?: string): number;
  has_theme_icon_override(name: string): boolean;
  has_theme_stylebox_override(name: string): boolean;
  has_theme_font_override(name: string): boolean;
  has_theme_font_size_override(name: string): boolean;
  has_theme_color_override(name: string): boolean;
  has_theme_constant_override(name: string): boolean;
  has_theme_icon(name: string, theme_type?: string): boolean;
  has_theme_stylebox(name: string, theme_type?: string): boolean;
  has_theme_font(name: string, theme_type?: string): boolean;
  has_theme_font_size(name: string, theme_type?: string): boolean;
  has_theme_color(name: string, theme_type?: string): boolean;
  has_theme_constant(name: string, theme_type?: string): boolean;
  get_theme_default_base_scale(): number;
  get_theme_default_font(): Font;
  get_theme_default_font_size(): number;
  get_window_id(): number;
  get_focused_window(): Window;
  set_layout_direction(direction: number): void;
  get_layout_direction(): number;
  is_layout_rtl(): boolean;
  set_use_font_oversampling(enable: boolean): void;
  is_using_font_oversampling(): boolean;
  popup(rect?: Rect2i): void;
  popup_on_parent(parent_rect: Rect2i): void;
  popup_centered(minsize?: Vector2i): void;
  popup_centered_ratio(ratio?: number): void;
  popup_centered_clamped(minsize?: Vector2i, fallback_ratio?: number): void;
  popup_exclusive(from_node: Node, rect?: Rect2i): void;
  popup_exclusive_on_parent(from_node: Node, parent_rect: Rect2i): void;
  popup_exclusive_centered(from_node: Node, minsize?: Vector2i): void;
  popup_exclusive_centered_ratio(from_node: Node, ratio?: number): void;
  popup_exclusive_centered_clamped(from_node: Node, minsize?: Vector2i, fallback_ratio?: number): void;
}

export declare class World2D extends Resource {
  readonly canvas: RID;
  readonly navigation_map: RID;
  readonly space: RID;
  readonly direct_space_state: PhysicsDirectSpaceState2D;
}

export declare class World3D extends Resource {
  environment: Environment;
  fallback_environment: Environment;
  readonly space: RID;
  readonly navigation_map: RID;
  readonly scenario: RID;
  readonly direct_space_state: PhysicsDirectSpaceState3D;
}

export declare class WorldBoundaryShape2D extends Shape2D {
  normal: Vector2;
  distance: number;
}

export declare class WorldBoundaryShape3D extends Shape3D {
  plane: Plane;
}

export declare class WorldEnvironment extends Node {
  environment: Environment;
  compositor: Compositor;
}

export declare class X509Certificate extends Resource {
  save(path: string): number;
  load(path: string): number;
  save_to_string(): string;
  load_from_string(string: string): number;
}

export declare class XMLParser extends RefCounted {
  read(): number;
  get_node_type(): number;
  get_node_name(): string;
  get_node_data(): string;
  get_node_offset(): number;
  get_attribute_count(): number;
  get_attribute_name(idx: number): string;
  get_attribute_value(idx: number): string;
  has_attribute(name: string): boolean;
  get_named_attribute_value(name: string): string;
  get_named_attribute_value_safe(name: string): string;
  is_empty(): boolean;
  get_current_line(): number;
  skip_section(): void;
  seek(position: number): number;
  open(file: string): number;
  open_buffer(buffer: PackedByteArray): number;
}

export declare class XRAnchor3D extends XRNode3D {
  get_size(): Vector3;
  get_plane(): Plane;
}

export declare class XRBodyModifier3D extends SkeletonModifier3D {
  body_update: number;
  bone_update: number;
}

export declare class XRBodyTracker extends XRPositionalTracker {
  has_tracking_data: boolean;
  body_flags: number;
  set_joint_flags(joint: number, flags: number): void;
  get_joint_flags(joint: number): number;
  set_joint_transform(joint: number, transform: Transform3D): void;
  get_joint_transform(joint: number): Transform3D;
}

export declare class XRCamera3D extends Camera3D {
}

export declare class XRController3D extends XRNode3D {
  is_button_pressed(name: string): boolean;
  get_input(name: string): any;
  get_float(name: string): number;
  get_vector2(name: string): Vector2;
  get_tracker_hand(): number;
}

export declare class XRControllerTracker extends XRPositionalTracker {
}

export declare class XRFaceModifier3D extends Node3D {
  target: string;
}

export declare class XRFaceTracker extends XRTracker {
  blend_shapes: PackedFloat32Array;
  get_blend_shape(blend_shape: number): number;
  set_blend_shape(blend_shape: number, weight: number): void;
}

export declare class XRHandModifier3D extends SkeletonModifier3D {
  bone_update: number;
}

export declare class XRHandTracker extends XRPositionalTracker {
  has_tracking_data: boolean;
  hand_tracking_source: number;
  set_hand_joint_flags(joint: number, flags: number): void;
  get_hand_joint_flags(joint: number): number;
  set_hand_joint_transform(joint: number, transform: Transform3D): void;
  get_hand_joint_transform(joint: number): Transform3D;
  set_hand_joint_radius(joint: number, radius: number): void;
  get_hand_joint_radius(joint: number): number;
  set_hand_joint_linear_velocity(joint: number, linear_velocity: Vector3): void;
  get_hand_joint_linear_velocity(joint: number): Vector3;
  set_hand_joint_angular_velocity(joint: number, angular_velocity: Vector3): void;
  get_hand_joint_angular_velocity(joint: number): Vector3;
}

export declare class XRInterface extends RefCounted {
  interface_is_primary: boolean;
  xr_play_area_mode: number;
  environment_blend_mode: number;
  ar_is_anchor_detection_enabled: boolean;
  get_name(): string;
  get_capabilities(): number;
  is_initialized(): boolean;
  initialize(): boolean;
  uninitialize(): void;
  get_system_info(): Record<string, any>;
  get_tracking_status(): number;
  get_render_target_size(): Vector2;
  get_view_count(): number;
  trigger_haptic_pulse(action_name: string, tracker_name: string, frequency: number, amplitude: number, duration_sec: number, delay_sec: number): void;
  supports_play_area_mode(mode: number): boolean;
  get_play_area(): PackedVector3Array;
  get_camera_feed_id(): number;
  is_passthrough_supported(): boolean;
  is_passthrough_enabled(): boolean;
  start_passthrough(): boolean;
  stop_passthrough(): void;
  get_transform_for_view(view: number, cam_transform: Transform3D): Transform3D;
  get_projection_for_view(view: number, aspect: number, near: number, far: number): Projection;
  get_supported_environment_blend_modes(): any[];
}

export declare class XRInterfaceExtension extends XRInterface {
  get_color_texture(): RID;
  get_depth_texture(): RID;
  get_velocity_texture(): RID;
  add_blit(render_target: RID, src_rect: Rect2, dst_rect: Rect2i, use_layer: boolean, layer: number, apply_lens_distortion: boolean, eye_center: Vector2, k1: number, k2: number, upscale: number, aspect_ratio: number): void;
  get_render_target_texture(render_target: RID): RID;
}

export declare class XRNode3D extends Node3D {
  show_when_tracked: boolean;
  get_is_active(): boolean;
  get_has_tracking_data(): boolean;
  trigger_haptic_pulse(action_name: string, frequency: number, amplitude: number, duration_sec: number, delay_sec: number): void;
}

export declare class XROrigin3D extends Node3D {
  world_scale: number;
  current: boolean;
}

export declare class XRPose extends RefCounted {
  has_tracking_data: boolean;
  tracking_confidence: number;
  get_adjusted_transform(): Transform3D;
}

export declare class XRPositionalTracker extends XRTracker {
  profile: string;
  hand: number;
  has_pose(name: string): boolean;
  get_pose(name: string): XRPose;
  invalidate_pose(name: string): void;
  set_pose(name: string, transform: Transform3D, linear_velocity: Vector3, angular_velocity: Vector3, tracking_confidence: number): void;
  get_input(name: string): any;
  set_input(name: string, value: any): void;
}

export declare class XRTracker extends RefCounted {
  type: number;
  description: string;
}

export declare class XRVRS extends Object {
  vrs_min_radius: number;
  vrs_strength: number;
  vrs_render_region: Rect2i;
  make_vrs_texture(target_size: Vector2, eye_foci: PackedVector2Array): RID;
}

export declare class ZIPPacker extends RefCounted {
  compression_level: number;
  open(path: string, append?: number): number;
  start_file(path: string): number;
  write_file(data: PackedByteArray): number;
  close_file(): number;
  close(): number;
}

export declare class ZIPReader extends RefCounted {
  open(path: string): number;
  close(): number;
  get_files(): PackedStringArray;
  read_file(path: string, case_sensitive?: boolean): PackedByteArray;
  file_exists(path: string, case_sensitive?: boolean): boolean;
  get_compression_level(path: string, case_sensitive?: boolean): number;
}

export declare const Performance: any;
export declare const Engine: any;
export declare const ProjectSettings: any;
export declare const OS: any;
export declare const Time: any;
export declare const TextServerManager: any;
export declare const PhysicsServer2DManager: any;
export declare const PhysicsServer3DManager: any;
export declare const NavigationMeshGenerator: any;
export declare const IP: any;
export declare const Geometry2D: any;
export declare const Geometry3D: any;
export declare const ResourceLoader: any;
export declare const ResourceSaver: any;
export declare const ClassDB: any;
export declare const Marshalls: any;
export declare const TranslationServer: any;
export declare const Input: any;
export declare const InputMap: any;
export declare const EngineDebugger: any;
export declare const GDExtensionManager: any;
export declare const ResourceUID: any;
export declare const WorkerThreadPool: any;
export declare const ThemeDB: any;
export declare const EditorInterface: any;
export declare const JavaClassWrapper: any;
export declare const JavaScriptBridge: any;
export declare const AudioServer: any;
export declare const CameraServer: any;
export declare const DisplayServer: any;
export declare const NativeMenu: any;
export declare const RenderingServer: any;
export declare const NavigationServer2D: any;
export declare const NavigationServer3D: any;
export declare const PhysicsServer2D: any;
export declare const PhysicsServer3D: any;
export declare const XRServer: any;

