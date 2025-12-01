# GodotJSRuntime - Sandbox

## 产品需求文档（PRD v3）

---

## 项目概述

构建一个**安全的 JavaScript/TypeScript 运行时沙箱**，允许玩家在 AI 辅助下，于游戏运行期间动态生成完整的游戏关卡、玩法逻辑，并持久化保存。

### 核心使用场景

```
玩家："帮我创建一个平台跳跃关卡，有浮动平台和金币收集"
    ↓
AI 生成 TypeScript 代码
    ↓
JSSandbox 即时执行，玩家预览效果
    ↓
玩家满意，保存关卡
    ↓
生成 .tscn 场景文件 + .js 行为脚本
    ↓
下次启动游戏，直接加载该关卡
```

---

## 项目目标

1. **完整的关卡生成能力**
   - 动态创建任意 Godot 节点和场景结构
   - 支持 `_ready()`, `_process()` 等生命周期回调
   - 信号连接与事件处理
   - 关卡持久化保存与加载

2. **原生脚本集成**
   - JS/TS 脚本可 attach 到节点（如同 GDScript）
   - 保存的 .tscn 文件可引用 .js 脚本
   - 支持 TypeScript（编译为 JS 执行）

3. **沙箱安全**
   - 阻止访问文件系统、网络、线程等危险 API
   - 限制执行时间和内存使用
   - JS 错误不会导致游戏崩溃

4. **内存安全**
   - JS 不接触原生指针
   - Godot 对象生命周期与 JS 同步
   - 防止访问已释放对象

---

## 技术架构

### 基于 GodotJS 核心提取

本项目从 GodotJS 项目提取运行时核心代码，去除编辑器相关功能，并添加沙箱安全层。

> **参考源码位置**：`./ref/GodotJS/`

```
GodotJS 完整项目                    本项目提取/新增
─────────────────                   ─────────────────
weaver-editor/    ← 不需要
bridge/           ← 提取 ──────────→ core/
  jsb_environment                     js_environment
  jsb_type_convert                    js_type_convert
  jsb_object_bindings                 js_object_bindings
  jsb_class_register                  js_class_register
weaver/           ← 简化提取 ──────→ script/
  jsb_script                          js_script
  jsb_script_instance                 js_script_instance
  jsb_script_language                 js_script_language (精简)
  jsb_resource_loader                 js_resource_loader
impl/quickjs/     ← 提取 ──────────→ impl/quickjs/
                                    sandbox/ (新增)
                                      safe_wrapper
                                      object_registry
                                      sandbox_config
                                      execution_limiter
```

### 模块结构

```
GodotJSRuntime/
├── quickjs/                 # QuickJS 引擎源码
├── core/                    # JS 运行时核心（从 GodotJS 提取）
│   ├── js_environment       # JS 执行环境
│   ├── js_type_convert      # Variant ↔ JSValue 转换
│   ├── js_object_bindings   # Godot 对象绑定
│   └── js_class_register    # 类注册系统
├── script/                  # Godot 脚本系统集成
│   ├── js_script            # Script 资源类型
│   ├── js_script_instance   # 脚本实例（处理 _ready/_process）
│   ├── js_script_language   # ScriptLanguageExtension（精简版）
│   └── js_resource_loader   # .js 文件加载器
├── sandbox/                 # 沙箱安全层（新增）
│   ├── safe_wrapper         # API 安全包装
│   ├── object_registry      # 对象生命周期管理
│   ├── sandbox_config       # 黑名单配置
│   └── execution_limiter    # 执行限制（时间/内存/频率）
├── persistence/             # 关卡持久化（新增）
│   ├── scene_saver          # 场景保存
│   └── script_saver         # 脚本保存
├── api/                     # 对外接口
│   └── js_sandbox           # JSSandbox 类（GDScript 调用）
└── generated/               # 自动生成的绑定代码
```

---

## 关卡生成完整流程

### 1. 即时执行与预览

```gdscript
# 游戏中的 AI 对话系统
var sandbox = JSSandbox.new()
sandbox.set_global("scene_root", $LevelRoot)
sandbox.set_global("player", $Player)

# AI 生成的代码即时执行
var code = ai_response.generated_code
var result = sandbox.eval(code)
```

### 2. 行为脚本示例

AI 生成的 TypeScript 代码：

```typescript
// floating_platform.ts
export default class FloatingPlatform extends StaticBody3D {
    @export amplitude: number = 1.0;
    @export speed: number = 1.0;

    private initial_y: number = 0;

    _ready() {
        this.initial_y = this.position.y;
    }

    _process(delta: number) {
        const time = Time.get_ticks_msec() / 1000.0;
        this.position.y = this.initial_y +
            Math.sin(time * this.speed) * this.amplitude;
    }
}
```

### 3. 保存关卡

```gdscript
# 玩家确认保存
sandbox.save_level("user://levels/my_level_1/")
```

生成的文件结构：

```
user://levels/my_level_1/
├── level.tscn              # 场景文件
├── floating_platform.js    # 编译后的 JS 脚本
├── coin_pickup.js          # 编译后的 JS 脚本
└── level.json              # 关卡元数据
```

**level.tscn 内容：**

```gdscript
[gd_scene load_steps=3 format=3]

[ext_resource type="Script" path="user://levels/my_level_1/floating_platform.js" id="1"]
[ext_resource type="Script" path="user://levels/my_level_1/coin_pickup.js" id="2"]

[node name="Level" type="Node3D"]

[node name="Platform1" type="StaticBody3D" parent="."]
script = ExtResource("1")
amplitude = 1.5
speed = 0.8

[node name="Coin" type="Area3D" parent="."]
script = ExtResource("2")
```

### 4. 加载关卡

```gdscript
# 下次启动游戏
var level = load("user://levels/my_level_1/level.tscn")
var instance = level.instantiate()
add_child(instance)
# JS 脚本自动执行，_ready/_process 正常工作
```

---

## 与 GodotJS 的关键差异

| 方面 | GodotJS | 本项目 |
|------|---------|--------|
| 定位 | 完整的 JS/TS 开发环境 | 运行时沙箱 |
| 编辑器集成 | 语法高亮、调试器、REPL | 不需要 |
| 安全性 | 无限制 | 黑名单 + 安全包装 |
| 文件访问 | 完全开放 | 仅 res:// 和 user:// |
| 网络访问 | 支持 | 禁止 |
| 使用场景 | 开发时 | 运行时（玩家/AI） |

---

## 目标用户

1. **游戏玩家**：通过 AI 对话创建自定义关卡
2. **Mod 作者**：用 JS/TS 编写游戏模组
3. **游戏开发者**：为游戏添加安全的脚本扩展能力

---

## 技术选型

### 1. JavaScript 引擎：QuickJS

| 考量因素   | QuickJS          | V8               | 结论         |
| ---------- | ---------------- | ---------------- | ------------ |
| 沙箱安全   | ✓ 无默认危险 API | 需要移除内置对象 | QuickJS 适合 |
| 二进制体积 | 极小             | 很大             | QuickJS ✓    |
| 启动速度   | 极快             | 较慢             | QuickJS ✓    |
| 内存占用   | 小               | 高               | QuickJS ✓    |
| 执行性能   | 中等             | 高               | 可接受       |

QuickJS 优点：可控、可注入、可限制、无 JIT、不越界写内存。

### 2. 插件方式：GDExtension

- 不修改引擎源码
- 原生性能
- 自动继承 Godot 多平台能力

---

# 沙箱设计核心理念（关键变化）

本版本设计明确采用：

**"最大 API 曝光 + 精准黑名单 + 强安全包装层 + 主线程执行"**

而不是白名单或多线程模型。

原因：

- Godot 安全 API 数量极大
- 玩家应能执行任何 Godot 机制
- 维护白名单成本爆炸
- 包装层（Wrapper）才是安全关键，而非 API 筛选
- QuickJS 是单线程库，在主线程执行可以直接调用 Godot API，避免复杂的跨线程同步

---

# 核心设计

## 1. 精准黑名单（仅屏蔽极少数 API）

黑名单不是用于限制 Godot 功能，而是屏蔽：

- 文件系统
- 网络
- 多线程
- 动态脚本执行
- 引擎单例访问
- 反射类 API（call, instance, instantiate 等）

示例：

```
[blocked_classes]
classes = [
    # 系统访问
    "OS",
    "FileAccess",
    "DirAccess",
    # 多线程
    "Thread",
    "Mutex",
    "Semaphore",
    "WorkerThreadPool",
    # 网络
    "TCPServer",
    "UDPServer",
    "HTTPClient",
    "HTTPRequest",
    "WebSocketPeer",
    "ENetConnection",
    "ENetMultiplayerPeer",
    # 脚本/代码执行
    "JavaScriptBridge",
    "Expression",           # 可执行任意 GDScript 代码
    "GDScript",             # 脚本类，防止动态创建
    "CSharpScript",         # C# 脚本类
    # 扩展/原生代码
    "NativeExtension",
    "GDExtensionManager",
    # 资源管理（通过安全的 load() 代替）
    "ResourceLoader",
    "ResourceSaver",
]

[blocked_methods]
Object = ["call", "callv", "set_script", "set", "get_script"]
ClassDB = ["instantiate", "instance", "can_instantiate", "get_class_list"]
Engine = ["get_singleton", "register_singleton", "unregister_singleton"]
Node = ["set_script"]  # 防止替换脚本绕过沙箱
```

说明：
- `call`/`callv`：防止绕过黑名单调用任意方法
- `set_script`：防止动态加载 GDScript 绕过沙箱
- `set`：防止通过属性名字符串设置任意属性

注意：

**queue_free 不屏蔽！必须通过包装层安全支持。**

---

## 2. 安全包装层（Safe Wrapper Layer）——真正的沙箱核心

JavaScript 调用 Godot 不是直接 API → C++，而是：

```
JS API → Wrapper → 验证 + 安全检查 → Godot 调用（主线程同步）
```

**线程模型：JS 在主线程执行**

QuickJS 是单线程库，JS 代码在 Godot 主线程执行，可以直接调用 Godot API，无需跨线程队列。

优点：
- 简化架构，无跨线程同步问题
- 直接调用 Godot API，无延迟
- 避免死锁风险

如果担心 JS 阻塞主线程，通过以下机制保护：
- interrupt handler 限制执行时间
- 指令计数限制防止死循环

包装层职责：

1. 验证对象是否有效（ObjectID 存活检查）
2. JS 对象与 Godot 实例 lifetime 同步
3. 捕获 Godot 抛出的所有异常
4. 自动清理 JS 回调（特别是信号）
5. 对关键 API（queue_free）做特殊生命周期处理

**包装层是整个系统安全的核心，不是黑名单。**

---

## 3. 资源加载安全（load 函数）

JS 中提供 `load()` 函数加载资源：

```javascript
sprite.texture = load("res://icon.png");
let scene = load("user://generated/enemy.tscn");
```

**路径限制：**

| 路径前缀 | 允许 | 说明 |
|---------|------|------|
| `res://` | ✓ | 游戏内置资源 |
| `user://` | ✓ | 用户数据目录（AI 生成的资源存放位置） |
| 绝对路径 | ✗ | 如 `/home/...` 或 `C:\...` |
| `../` | ✗ | 目录穿越 |

**资源类型限制：**

| 类型 | 允许的扩展名 | 说明 |
|-----|-------------|------|
| 场景 | `.tscn`, `.scn` | 场景文件 |
| 图片 | `.png`, `.jpg`, `.jpeg`, `.webp`, `.svg` | 纹理资源 |
| 音频 | `.ogg`, `.wav`, `.mp3` | 音效/音乐 |
| 3D 模型 | `.glb`, `.gltf` | 模型文件 |
| 资源 | `.tres`, `.material` | Godot 资源 |
| 字体 | `.ttf`, `.otf` | 字体文件 |
| 脚本 | `.js` | 仅 JS 脚本 |

**禁止加载的类型：**
- `.gd` (GDScript)
- `.cs` (C#)
- `.so`, `.dll`, `.dylib` (原生库)
- `.gdextension` (扩展)

实现要点：
```cpp
JSValue js_load(JSContext* ctx, const char* path) {
    String p = String(path);

    // 路径安全检查
    if (!p.begins_with("res://") && !p.begins_with("user://")) {
        return JS_ThrowTypeError(ctx, "Only res:// and user:// paths allowed");
    }
    if (p.find("..") != -1) {
        return JS_ThrowTypeError(ctx, "Path traversal not allowed");
    }

    // 资源类型检查
    String ext = p.get_extension().to_lower();
    if (is_blocked_extension(ext)) {
        return JS_ThrowTypeError(ctx, "Resource type not allowed: %s", ext.utf8().get_data());
    }

    // 安全加载资源...
}
```

---

## 4. Godot 对象生命周期安全

采用：

**ObjectRegistry（对象句柄池） + ObjectID + Generation/version 机制**

JS 对象：

```
{ id: 12345, version: 7 }
```

Godot 对象删除时：

- Node：监听 NOTIFICATION_PREDELETE
- RefCounted：通过引用计数管理（详见下文）

**RefCounted 对象生命周期管理：**

```
JS 获取 RefCounted 对象时：
1. ObjectRegistry.create_handle(obj)
2. obj->reference()  // 增加引用计数，防止被释放

JS GC 回收 wrapper 时：
1. release callback 被调用
2. obj->unreference()  // 减少引用计数
3. 如果引用计数归零，对象自动释放
4. ObjectRegistry.mark_deleted(id)
```

**信号（Signal）安全机制：**

JS 可以连接和断开信号，返回连接对象：

```javascript
// 连接信号，返回 SignalConnection 对象
let connection = player.connect("health_changed", (new_health) => {
    console.log("Health:", new_health);
});

// 检查连接状态
if (connection.is_connected()) {
    // 断开信号
    connection.disconnect();
}
```

信号管理要点：

1. **回调注册**：JS 回调函数存储在 ObjectRegistry 中，关联到目标对象
2. **弱引用语义**：信号连接不增加目标对象的引用计数
3. **自动清理**：当发射信号的对象被删除时，自动断开所有 JS 回调
4. **沙箱销毁清理**：JSSandbox 析构时断开所有信号连接，防止悬空回调

清理过程：

1. 从 ObjectRegistry 中标记无效
2. 断开该对象相关的所有信号回调
3. JS 访问该对象会抛出异常，不会崩溃

此机制属于工业级脚本安全处理方案。

---

## 5. queue_free 的安全实现（核心）

JavaScript 中：

```javascript
node.queue_free();
```

实际执行流程（JS 在主线程同步执行）：

1. SafeWrapper 验证对象有效性
2. 直接调用 node->queue_free()
3. Godot 在帧结束时真正删除节点
4. NOTIFICATION_PREDELETE 触发 → ObjectRegistry 标记 id 为 invalid
5. 断开该对象相关的所有信号回调
6. JS 后续访问时抛出错误（但不会崩）

这样：

- 功能完整（玩家能自由删除节点）
- 内存安全（不会访问已删除对象）
- 生命周期一致
- AI 可以动态更新场景树

---

## 5. 错误隔离（不允许崩溃）

所有 JS 执行：

- 包裹错误处理
- 捕获异常 → 转换为 Godot error
- 不允许 API 崩溃引擎

例如：

JS 调用 Godot 时产生报错：

- 方法参数错误
- 对象已无效
- 节点不在树中

全部变成 JS 异常，而不是 crash。

---

## 6. 执行限制（安全保障）

### （1）内存限制

```
JS_SetMemoryLimit(rt, 64 * 1024 * 1024)
```

避免 DoS。

### （2）时间片中断（timeout）

每段脚本执行设置 deadline，通过 interrupt handler 终止。

### （3）指令计数限制

对死循环加保险：

```
每 N 个字节码检查一次中断
```

### （4）执行限制（三层保护）

采用时间片 + 分级调用限制的混合方案：

**第一层：时间片限制（主要）**
```
单次 eval() 最大执行时间：1000ms（可配置）
单帧 _process() 最大执行时间：16ms（约 1 帧）
```

**第二层：API 调用分级限制（辅助）**
```
读操作（get_position 等）：不限制
写操作（set_position、add_child 等）：每帧最多 500 次
重操作（instantiate、queue_free 等）：每帧最多 50 次
```

**第三层：指令计数（兜底）**
```
每 10000 条字节码检查一次中断
```

防止 API 型 DoS 例如：

for(...) node.set_position(...);

---

## 7. API 绑定方式：离线生成（BUILD TIME）

**不在启动时使用 ClassDB 动态扫描！**

构建流程：

1. 执行 Godot Editor 的绑定生成脚本
2. 解析 ClassDB
3. 将所有 Godot API 自动生成 → C++ Binding + JS Wrapper
4. 输出到 `generated/` 文件夹
5. 沙箱运行时直接加载这些预生成绑定

好处：

- 启动速度快
- 不依赖反射
- 更安全（可以做黑名单过滤）
- 维护成本低
- 自动适配引擎版本

---

## 8. 类型系统支持（Variant ↔ JS）

类型映射：

- 基础类型
- 数学类型 (Vector2, Vector3, Color…)
- Array ↔ JS Array
- Dictionary ↔ JS Object
- Object ↔ JS Wrapper

不会暴露任何原生指针。

---

## 实现阶段计划

### Phase 1：GodotJS 核心提取与精简

- 从 GodotJS 提取 QuickJS 实现层
- 提取类型转换系统（js_type_convert）
- 提取对象绑定系统（js_object_bindings）
- 去除编辑器相关代码
- 验证基础 eval() 功能

### Phase 2：ScriptLanguage 集成

- 提取并简化 js_script_language
- 实现 js_script（Script 资源类型）
- 实现 js_script_instance（处理 _ready/_process）
- 注册 .js 文件扩展名
- 验证脚本 attach 到节点功能

### Phase 3：沙箱安全层

- 实现 SafeWrapper（API 安全包装）
- 实现 ObjectRegistry（对象生命周期管理）
- 实现黑名单配置加载
- 实现执行限制（时间/内存/频率）
- Hook 所有 API 调用路径

### Phase 4：关卡持久化

- 实现场景序列化（PackedScene 保存）
- 实现脚本文件保存（.js 写入 user://）
- 实现关卡元数据保存
- 路径安全检查（只允许 user:// 写入）

### Phase 5：TypeScript 支持

- 集成 TypeScript 编译器（可选：运行时编译或预编译）
- @export 装饰器支持
- 类型定义文件生成

> **注意**：Phase 1-4 阶段暂不支持 TypeScript，AI 生成的代码应为纯 JavaScript（ES6+）。
> 文档中的 TypeScript 示例（class 语法、箭头函数等）在 ES6 JavaScript 中同样有效。
> @export 装饰器在早期阶段可用 `static __exports__ = {}` 替代。

### Phase 6：API 完善与测试

- JSSandbox 对外接口完善
- 完整的错误处理和日志
- 单元测试和集成测试
- 性能优化

---

# 对外 API 设计

## JSSandbox 类

```gdscript
class_name JSSandbox extends RefCounted

# ===== 配置 =====
func set_timeout_ms(ms: int) -> void
func set_memory_limit_mb(mb: int) -> void
func load_blocklist(path: String) -> Error

# ===== 执行 =====
func eval(code: String) -> Variant
func eval_file(path: String) -> Variant

# ===== 全局变量 =====
func set_global(name: String, value: Variant) -> void
func get_global(name: String) -> Variant

# ===== 关卡持久化 =====
func save_level(directory: String) -> Error
func get_created_nodes() -> Array[Node]
func get_attached_scripts() -> Dictionary  # {Node: script_source}

# ===== 信号 =====
signal error_occurred(message: String, line: int, column: int)
signal console_output(message: String)
signal level_saved(path: String)
```

## 使用示例

### 基础执行

```gdscript
var sandbox = JSSandbox.new()
sandbox.set_timeout_ms(1000)
sandbox.set_memory_limit_mb(64)
sandbox.set_global("scene", $LevelRoot)

var result = sandbox.eval("""
    let platform = new StaticBody3D();
    platform.position = new Vector3(0, 5, 0);
    scene.add_child(platform);
""")
```

### AI 辅助关卡生成

```gdscript
# AI 生成带行为的节点
sandbox.eval("""
    // 创建浮动平台类
    class FloatingPlatform extends StaticBody3D {
        _ready() {
            this.initial_y = this.position.y;
        }
        _process(delta) {
            this.position.y = this.initial_y + Math.sin(Time.get_ticks_msec() / 1000) * 2;
        }
    }

    // 创建实例
    let platform = new FloatingPlatform();
    platform.position = new Vector3(0, 5, 0);
    scene.add_child(platform);
""")

# 玩家满意后保存
sandbox.save_level("user://levels/my_level/")
```

### 加载保存的关卡

```gdscript
# 下次游戏启动时
var level = load("user://levels/my_level/level.tscn")
add_child(level.instantiate())
# JS 脚本自动执行
```

---
