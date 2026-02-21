#!/usr/bin/env python3
"""
Generate EmmyLua-annotated stub files from Octave Engine C++ Lua bindings.

Parses *_Lua.h and *_Lua.cpp files to produce .lua stubs that LuaLS can
consume for IntelliSense (autocomplete, param hints, type checking).

Usage:
    python Tools/generate_lua_stubs.py
    python Tools/generate_lua_stubs.py --input Engine/Source/LuaBindings --output Engine/Generated/LuaMeta --verbose
"""

import argparse
import os
import re
import sys
from dataclasses import dataclass, field
from typing import Optional


# ---------------------------------------------------------------------------
# CHECK_* macro -> LuaLS type mapping
# ---------------------------------------------------------------------------

# Primitives from LuaTypeCheck.h
PRIMITIVE_CHECK_MAP = {
    "CHECK_NUMBER":   "number",
    "CHECK_STRING":   "string",
    "CHECK_BOOLEAN":  "boolean",
    "CHECK_INTEGER":  "integer",
    "CHECK_INDEX":    "integer",
    "CHECK_TABLE":    "table",
    "CHECK_FUNCTION": "function",
    "CHECK_USERDATA": "userdata",
    "CHECK_NIL":      "nil",
}

# Input codes treated as integer
INPUT_CHECK_MAP = {
    "CHECK_KEY":          "integer",
    "CHECK_MOUSE":        "integer",
    "CHECK_GAMEPAD":      "integer",
    "CHECK_GAMEPAD_AXIS": "integer",
}

# Engine types: CHECK_<TYPE> -> Lua class name
# Built dynamically from headers, but we seed with known ones
ENGINE_CHECK_MAP = {
    "CHECK_VECTOR":     "Vector",
    "CHECK_RECT":       "Rect",
    "CHECK_NODE":       "Node",
    "CHECK_NODE_PTR":   "Node",
    "CHECK_WORLD":      "World",
    "CHECK_STREAM":     "Stream",
    "CHECK_SIGNAL":     "Signal",
}

# lua_push* -> LuaLS return type
PUSH_TYPE_MAP = {
    "lua_pushboolean":  "boolean",
    "lua_pushnumber":   "number",
    "lua_pushinteger":  "integer",
    "lua_pushstring":   "string",
    "lua_pushfstring":  "string",
    "lua_pushnil":      "nil",
}

# *_Lua::Create -> LuaLS return type
CREATE_TYPE_MAP = {
    "Vector_Lua::Create": "Vector",
    "Rect_Lua::Create":   "Rect",
    "Node_Lua::Create":   "Node",
    "World_Lua::Create":  "World",
    "Stream_Lua::Create": "Stream",
    "Signal_Lua::Create": "Signal",
}

# Metamethods to skip in stub output (internal C machinery)
SKIP_METAMETHODS = {"__gc", "__index", "__newindex", "__tostring", "__len"}

# Operator metamethods that get special annotations
OPERATOR_METAMETHODS = {
    "__add": "Addition operator (+)",
    "__sub": "Subtraction operator (-)",
    "__mul": "Multiplication operator (*)",
    "__div": "Division operator (/)",
    "__mod": "Modulo operator (%)",
    "__unm": "Unary minus operator (-)",
    "__eq":  "Equality operator (==)",
    "__lt":  "Less-than operator (<)",
    "__le":  "Less-than-or-equal operator (<=)",
    "__concat": "Concatenation operator (..)",
}


# ---------------------------------------------------------------------------
# Data classes
# ---------------------------------------------------------------------------

@dataclass
class ParamInfo:
    name: str
    lua_type: str
    optional: bool = False


@dataclass
class MethodInfo:
    """A single Lua-exposed method."""
    name: str            # Lua-facing name
    cpp_func: str        # C++ function name (may differ from name for aliases)
    params: list         # list[ParamInfo]
    returns: list        # list[str] - LuaLS type names
    is_alias: bool = False


@dataclass
class EnumEntry:
    name: str            # Field name in Lua
    # We don't know actual int values at parse time, just emit 0


@dataclass
class EnumInfo:
    global_name: str
    entries: list        # list[EnumEntry]


@dataclass
class TypeInfo:
    """A bound Lua type (class, module, value-type, or enum)."""
    lua_name: str
    kind: str            # "class", "module", "value"
    parent: Optional[str] = None
    methods: list = field(default_factory=list)    # list[MethodInfo]
    fields: list = field(default_factory=list)     # list[(name, type)]
    operators: list = field(default_factory=list)  # list[(op, desc)]
    source_file: str = ""
    extra_globals: list = field(default_factory=list)  # list[(name, target)]


# ---------------------------------------------------------------------------
# Parsing helpers
# ---------------------------------------------------------------------------

def read_file(path):
    """Read a file and return its contents, handling encoding issues."""
    for enc in ("utf-8", "latin-1"):
        try:
            with open(path, "r", encoding=enc) as f:
                return f.read()
        except UnicodeDecodeError:
            continue
    return ""


def extract_function_bodies(source):
    """
    Extract all top-level C function bodies from source text.
    Returns dict mapping "ClassName::FuncName" or "FuncName" to the full body text.
    """
    bodies = {}
    # Match: int/void ClassName::FuncName(...) {   OR   int/void FuncName(...) {
    func_pattern = re.compile(
        r'^(?:int|void)\s+(\w+(?:::\w+)?)\s*\([^)]*\)\s*\{',
        re.MULTILINE
    )
    for m in func_pattern.finditer(source):
        func_name = m.group(1)
        start = m.end() - 1  # position of opening brace
        body = _extract_brace_block(source, start)
        if body is not None:
            bodies[func_name] = body
    return bodies


def _extract_brace_block(source, start):
    """Extract text between matched braces starting at source[start] == '{'."""
    if start >= len(source) or source[start] != '{':
        return None
    depth = 0
    i = start
    while i < len(source):
        ch = source[i]
        if ch == '{':
            depth += 1
        elif ch == '}':
            depth -= 1
            if depth == 0:
                return source[start:i+1]
        elif ch == '/' and i + 1 < len(source):
            if source[i+1] == '/':
                # Skip line comment
                i = source.find('\n', i)
                if i == -1:
                    break
            elif source[i+1] == '*':
                # Skip block comment
                end = source.find('*/', i + 2)
                if end == -1:
                    break
                i = end + 1
        elif ch == '"':
            # Skip string literal
            i += 1
            while i < len(source) and source[i] != '"':
                if source[i] == '\\':
                    i += 1
                i += 1
        elif ch == "'":
            # Skip char literal
            i += 1
            while i < len(source) and source[i] != "'":
                if source[i] == '\\':
                    i += 1
                i += 1
        i += 1
    return None


def parse_check_macros_from_body(body, is_self_type):
    """
    Parse CHECK_* macro calls from a function body to determine parameters.
    Returns list[ParamInfo].

    is_self_type: if True, skip arg index 1 (it's the self/object argument).
    """
    params = []
    seen_args = {}  # arg_index -> ParamInfo

    # Find all CHECK_*(L, N) calls - capture the macro name and arg index
    check_pattern = re.compile(r'(CHECK_\w+)\s*\(\s*L\s*,\s*(\d+)\s*\)')
    for m in check_pattern.finditer(body):
        macro = m.group(1)
        arg_idx = int(m.group(2))

        # Skip self argument for class/value methods
        if is_self_type and arg_idx == 1:
            continue

        if arg_idx in seen_args:
            continue  # Already found this arg

        lua_type = _macro_to_lua_type(macro)
        if lua_type is None:
            continue

        # Try to infer parameter name from C++ variable assignment
        param_name = _infer_param_name(body, m.start(), macro, arg_idx, is_self_type)

        # Check if this parameter is optional (guarded by lua_isnone)
        is_optional = _is_optional_param(body, arg_idx)

        seen_args[arg_idx] = ParamInfo(
            name=param_name,
            lua_type=lua_type,
            optional=is_optional
        )

    # Also detect optional params that use lua_isinteger/lua_isnumber patterns
    # like: int index = lua_isinteger(L, 2) ? lua_tointeger(L, 2) - 1 : 0;
    optional_pattern = re.compile(r'lua_is(?:integer|number)\s*\(\s*L\s*,\s*(\d+)\s*\)\s*\?')
    for m in optional_pattern.finditer(body):
        arg_idx = int(m.group(1))
        if is_self_type and arg_idx == 1:
            continue
        if arg_idx not in seen_args:
            # Infer name from the variable assignment
            line_start = body.rfind('\n', 0, m.start()) + 1
            line = body[line_start:body.find('\n', m.start())]
            name_match = re.match(r'\s*(?:\w+\s+)+(\w+)\s*=', line)
            param_name = name_match.group(1) if name_match else f"arg{arg_idx}"
            # Determine type from lua_isinteger vs lua_isnumber
            if 'lua_isinteger' in m.group(0):
                lua_type = "integer"
            else:
                lua_type = "number"
            seen_args[arg_idx] = ParamInfo(
                name=param_name,
                lua_type=lua_type,
                optional=True
            )

    # Sort by arg index
    sorted_indices = sorted(seen_args.keys())
    return [seen_args[i] for i in sorted_indices]


def _macro_to_lua_type(macro):
    """Map a CHECK_* macro name to a LuaLS type string."""
    if macro in PRIMITIVE_CHECK_MAP:
        return PRIMITIVE_CHECK_MAP[macro]
    if macro in INPUT_CHECK_MAP:
        return INPUT_CHECK_MAP[macro]
    if macro in ENGINE_CHECK_MAP:
        return ENGINE_CHECK_MAP[macro]
    return None


def _infer_param_name(body, check_pos, macro, arg_idx, is_self_type):
    """Try to extract the C++ variable name from the assignment containing CHECK_*."""
    # Look at the line containing the CHECK call
    line_start = body.rfind('\n', 0, check_pos) + 1
    line_end = body.find('\n', check_pos)
    if line_end == -1:
        line_end = len(body)
    line = body[line_start:line_end]

    # Pattern: type varName = CHECK_*(L, N);
    # Also handles: varName = (cast)CHECK_*(L, N); (inside if blocks etc.)
    assign_match = re.match(r'\s*(?:[\w:*&<>]+\s+)+(\w+)\s*=\s*' + re.escape(macro), line)
    if not assign_match:
        # Try: varName = (cast)CHECK_*(L, N) - assignment without type declaration
        assign_match = re.search(r'(\w+)\s*=\s*(?:\([^)]*\)\s*)?' + re.escape(macro), line)
    if assign_match:
        name = assign_match.group(1)
        # Skip C++ prefixes/common names that aren't meaningful
        if name not in ('comp', 'node', 'widget', 'asset', 'ret', 'result', 'self'):
            return name

    # Fallback: generate from arg index
    offset = 1 if is_self_type else 0
    return f"arg{arg_idx - offset}"


def _is_optional_param(body, arg_idx):
    """Check if parameter at arg_idx is guarded by lua_isnone or similar."""
    # Pattern: if (!lua_isnone(L, N))
    pattern = re.compile(
        r'(?:lua_isnone|lua_isnil)\s*\(\s*L\s*,\s*' + str(arg_idx) + r'\s*\)'
    )
    return pattern.search(body) is not None


def parse_return_types(body):
    """
    Determine return types from lua_push* calls and *_Lua::Create calls
    just before `return N` statements.
    """
    returns = []

    # Find the return statement(s) - specifically `return N;` where N > 0
    return_match = re.search(r'return\s+(\d+)\s*;', body)
    if not return_match:
        return []

    return_count = int(return_match.group(1))
    if return_count == 0:
        return []

    # Scan for push/create calls in the body
    found_returns = []

    # lua_push* calls
    for push_func, lua_type in PUSH_TYPE_MAP.items():
        if push_func + '(' in body:
            found_returns.append(lua_type)

    # *_Lua::Create calls
    for create_func, lua_type in CREATE_TYPE_MAP.items():
        if create_func + '(' in body:
            found_returns.append(lua_type)

    # LuaPushDatum -> table
    if 'LuaPushDatum(' in body:
        found_returns.append("table")

    if not found_returns:
        return ["any"] * return_count

    # If we have exactly the right number, great
    if len(found_returns) == return_count:
        return found_returns

    # If we found more, try to be smarter by ordering them by position
    if len(found_returns) > return_count:
        return _order_returns_by_position(body, return_count)

    # If we found fewer, pad with any
    while len(found_returns) < return_count:
        found_returns.append("any")
    return found_returns[:return_count]


def _order_returns_by_position(body, count):
    """Order return types by their position in the function body."""
    positions = []

    for push_func, lua_type in PUSH_TYPE_MAP.items():
        for m in re.finditer(re.escape(push_func) + r'\s*\(', body):
            positions.append((m.start(), lua_type))

    for create_func, lua_type in CREATE_TYPE_MAP.items():
        for m in re.finditer(re.escape(create_func) + r'\s*\(', body):
            positions.append((m.start(), lua_type))

    if 'LuaPushDatum(' in body:
        for m in re.finditer(r'LuaPushDatum\s*\(', body):
            positions.append((m.start(), "table"))

    # Sort by position, take the last N (closest to return statement)
    positions.sort(key=lambda x: x[0])
    if len(positions) >= count:
        return [t for _, t in positions[-count:]]
    return [t for _, t in positions] + ["any"] * (count - len(positions))


# ---------------------------------------------------------------------------
# Bind() function parsing
# ---------------------------------------------------------------------------

def parse_bind_function(source, class_prefix):
    """
    Parse the Bind() function of a *_Lua.cpp file.
    Returns (kind, parent_name, global_name, registrations, extra_globals).

    kind: "class", "module", or "value"
    parent_name: parent class Lua name or None
    global_name: the lua_setglobal name
    registrations: list of (lua_name, cpp_func_name) tuples
    extra_globals: list of (global_name, cpp_func_name) for extra global shortcuts
    """
    # Extract the Bind() function body
    bind_key = f"{class_prefix}::Bind"
    bodies = extract_function_bodies(source)
    bind_body = bodies.get(bind_key)
    if bind_body is None:
        return None, None, None, [], []

    # Determine kind
    kind = "module"
    parent_name = None
    class_name_const = None  # The LUA_NAME constant for class types

    # Check for CreateClassMetatable
    class_mt_match = re.search(
        r'CreateClassMetatable\s*\(\s*'
        r'(\w+)\s*,\s*'     # className
        r'(\w+)\s*,\s*'     # classFlag
        r'(\w+|nullptr)\s*\)',  # parentClassName
        bind_body
    )
    if class_mt_match:
        kind = "class"
        class_name_const = class_mt_match.group(1)  # e.g. NODE_LUA_NAME
        parent_arg = class_mt_match.group(3)
        if parent_arg != "nullptr":
            parent_name = parent_arg  # This is a #define constant, resolve later
    elif 'luaL_newmetatable' in bind_body:
        kind = "value"

    # Extract registered functions
    registrations = []

    # REGISTER_TABLE_FUNC(L, idx, FuncName)
    for m in re.finditer(r'REGISTER_TABLE_FUNC\s*\(\s*L\s*,\s*\w+\s*,\s*(\w+)\s*\)', bind_body):
        func_name = m.group(1)
        registrations.append((func_name, func_name))

    # REGISTER_TABLE_FUNC_EX(L, idx, FuncName, "AliasName")
    for m in re.finditer(r'REGISTER_TABLE_FUNC_EX\s*\(\s*L\s*,\s*\w+\s*,\s*(\w+)\s*,\s*"([^"]+)"\s*\)', bind_body):
        cpp_func = m.group(1)
        alias_name = m.group(2)
        registrations.append((alias_name, cpp_func))

    # Extract global name from lua_setglobal
    global_name = None

    # For class types, the name is the first arg of CreateClassMetatable
    # (CreateClassMetatable internally calls lua_setglobal with className)
    if class_name_const:
        global_name = class_name_const  # Resolve via name_map later

    # For module/value types, look for explicit lua_setglobal
    if global_name is None:
        # Via constant: lua_setglobal(L, SOMETHING_LUA_NAME)
        global_match = re.search(r'lua_setglobal\s*\(\s*L\s*,\s*(\w+_LUA_NAME)\s*\)', bind_body)
        if global_match:
            global_name = global_match.group(1)  # Resolve later
        else:
            # Direct string (but NOT ones preceded by lua_pushcfunction, those are extra globals)
            # Find all lua_setglobal with direct strings
            all_setglobals = list(re.finditer(
                r'lua_setglobal\s*\(\s*L\s*,\s*"(\w+)"\s*\)', bind_body
            ))
            # Filter out any that are part of a pushcfunction+setglobal pair
            extra_global_positions = set()
            extra_pattern = re.compile(
                r'lua_pushcfunction\s*\(\s*L\s*,\s*\w+::\w+\s*\)\s*;\s*'
                r'lua_setglobal\s*\(\s*L\s*,\s*"(\w+)"\s*\)',
                re.DOTALL
            )
            for em in extra_pattern.finditer(bind_body):
                extra_global_positions.add(em.start())

            for sg in all_setglobals:
                # Check this isn't part of a pushcfunction pair by seeing if
                # there's a lua_pushcfunction shortly before
                preceding = bind_body[max(0, sg.start() - 200):sg.start()]
                if 'lua_pushcfunction' not in preceding:
                    global_name = sg.group(1)
                    break

    # Extra global shortcuts (e.g., lua_pushcfunction + lua_setglobal outside the main table)
    extra_globals = []
    # Pattern: lua_pushcfunction(L, Xxx_Lua::Func); lua_setglobal(L, "Name");
    extra_pattern = re.compile(
        r'lua_pushcfunction\s*\(\s*L\s*,\s*(\w+::\w+)\s*\)\s*;\s*'
        r'lua_setglobal\s*\(\s*L\s*,\s*"(\w+)"\s*\)',
        re.DOTALL
    )
    for m in extra_pattern.finditer(bind_body):
        extra_globals.append((m.group(2), m.group(1)))

    return kind, parent_name, global_name, registrations, extra_globals


def parse_bind_common(source, class_prefix):
    """Parse BindCommon() to get additional registered methods."""
    bind_key = f"{class_prefix}::BindCommon"
    bodies = extract_function_bodies(source)
    bind_body = bodies.get(bind_key)
    if bind_body is None:
        return []

    registrations = []
    for m in re.finditer(r'REGISTER_TABLE_FUNC\s*\(\s*L\s*,\s*\w+\s*,\s*(\w+)\s*\)', bind_body):
        func_name = m.group(1)
        registrations.append((func_name, func_name))
    for m in re.finditer(r'REGISTER_TABLE_FUNC_EX\s*\(\s*L\s*,\s*\w+\s*,\s*(\w+)\s*,\s*"([^"]+)"\s*\)', bind_body):
        registrations.append((m.group(2), m.group(1)))
    return registrations


def parse_enum_functions(source):
    """
    Parse enum-style binding functions (Misc_Lua.cpp, Input_Lua.cpp BindXxxTable).
    Returns list[EnumInfo].
    """
    enums = []

    # Find all functions that contain lua_newtable + lua_pushinteger + lua_setfield + lua_setglobal
    bodies = extract_function_bodies(source)

    for func_name, body in bodies.items():
        # Must have lua_setglobal to be an enum table
        global_match = re.search(r'lua_setglobal\s*\(\s*L\s*,\s*"(\w+)"\s*\)', body)
        if not global_match:
            continue

        # Must have lua_pushinteger + lua_setfield pairs
        entries = []
        for m in re.finditer(r'lua_setfield\s*\(\s*L\s*,\s*\w+\s*,\s*"([^"]+)"\s*\)', body):
            field_name = m.group(1)
            # Check that the preceding line has lua_pushinteger
            preceding = body[max(0, m.start() - 200):m.start()]
            if 'lua_pushinteger' in preceding:
                entries.append(EnumEntry(name=field_name))

        if entries:
            # Deduplicate entries (can happen from #if PLATFORM_3DS branches)
            seen_names = set()
            deduped = []
            for entry in entries:
                if entry.name not in seen_names:
                    deduped.append(entry)
                    seen_names.add(entry.name)
            enum_global = global_match.group(1)
            enums.append(EnumInfo(global_name=enum_global, entries=deduped))

    return enums


# ---------------------------------------------------------------------------
# Header parsing
# ---------------------------------------------------------------------------

def parse_headers(input_dir):
    """
    Parse all *_Lua.h headers for LUA_NAME, LUA_FLAG constants and CHECK_* macros.
    Returns (name_map, flag_map, check_map).

    name_map: {DEFINE_NAME: "LuaName"} e.g. {"CAMERA_3D_LUA_NAME": "Camera3D"}
    flag_map: {DEFINE_NAME: "cfFlag"}
    check_map: {CHECK_MACRO: "LuaType"} additional engine type CHECK_* macros
    """
    name_map = {}
    flag_map = {}
    check_map = {}

    for fname in sorted(os.listdir(input_dir)):
        if not fname.endswith('_Lua.h') and fname != 'LuaTypeCheck.h':
            continue

        path = os.path.join(input_dir, fname)
        content = read_file(path)

        # Parse #define XXX_LUA_NAME "YYY"
        for m in re.finditer(r'#define\s+(\w+_LUA_NAME)\s+"(\w+)"', content):
            name_map[m.group(1)] = m.group(2)

        # Parse #define XXX_LUA_FLAG "YYY"
        for m in re.finditer(r'#define\s+(\w+_LUA_FLAG)\s+"(\w+)"', content):
            flag_map[m.group(1)] = m.group(2)

        # Parse CHECK_* macros that map to engine types
        # Pattern: #define CHECK_XXX(L, arg) CheckLuaType<XXX_Lua>(L, arg, ...
        # or: #define CHECK_XXX(L, arg) CheckHierarchyLuaType<XXX>(L, arg, ...
        # or: #define CHECK_XXX(L, arg) CheckAssetLuaType<XXX>(L, arg, ...
        # or: #define CHECK_XXX(L, arg) static_cast<XXX*>(CheckNodeLuaType(...
        for m in re.finditer(
            r'#define\s+(CHECK_\w+)\s*\([^)]+\)\s+'
            r'(?:static_cast<\w+\*?>?\()?'
            r'(?:Check\w+<(\w+?)>|CheckNodeLuaType|CheckNodeWrapper\w*)',
            content
        ):
            macro_name = m.group(1)
            if macro_name not in PRIMITIVE_CHECK_MAP and macro_name not in INPUT_CHECK_MAP:
                # Determine the Lua type name from the header's LUA_NAME
                lua_type = None
                for define_name, lua_name in name_map.items():
                    # Match by file context
                    if define_name in content:
                        lua_type = lua_name
                        break
                if lua_type and macro_name not in ENGINE_CHECK_MAP:
                    check_map[macro_name] = lua_type

    return name_map, flag_map, check_map


def build_check_map_from_headers(input_dir):
    """
    Build a comprehensive CHECK_* -> LuaType map by reading each header
    and associating its CHECK_ macro with its LUA_NAME.
    """
    check_map = {}
    check_map.update(ENGINE_CHECK_MAP)

    for fname in sorted(os.listdir(input_dir)):
        if not fname.endswith('_Lua.h'):
            continue

        path = os.path.join(input_dir, fname)
        content = read_file(path)

        # Find the LUA_NAME in this file
        lua_name_match = re.search(r'#define\s+\w+_LUA_NAME\s+"(\w+)"', content)
        if not lua_name_match:
            continue
        lua_name = lua_name_match.group(1)

        # Find CHECK_* macros in this file
        for m in re.finditer(r'#define\s+(CHECK_\w+)\s*\(', content):
            macro = m.group(1)
            if macro not in PRIMITIVE_CHECK_MAP and macro not in INPUT_CHECK_MAP:
                if macro not in check_map:
                    check_map[macro] = lua_name

    return check_map


# ---------------------------------------------------------------------------
# Main processing
# ---------------------------------------------------------------------------

def resolve_constant(name, name_map):
    """Resolve a #define constant name to its string value."""
    if name in name_map:
        return name_map[name]
    # Try direct string
    if not name.endswith('_LUA_NAME'):
        return name
    return None


def process_binding_file(filepath, name_map, check_map, verbose=False):
    """
    Process a single *_Lua.cpp file.
    Returns (list[TypeInfo], list[EnumInfo]).
    """
    source = read_file(filepath)
    basename = os.path.basename(filepath)
    class_prefix = basename.replace('.cpp', '').replace('_Lua', '_Lua')
    # e.g. "Camera3D_Lua" from "Camera3d_Lua.cpp" - handle the casing
    # Actually extract from the source itself
    class_prefix_match = re.search(r'(\w+_Lua)::Bind\b', source)
    if not class_prefix_match:
        # Try BindMisc or other patterns
        if 'Misc_Lua' in basename:
            return [], parse_enum_functions(source)
        return [], []

    class_prefix = class_prefix_match.group(1)

    if verbose:
        print(f"  Processing {basename} (class: {class_prefix})")

    # Parse the Bind() function
    kind, parent_const, global_const, registrations, extra_globals = parse_bind_function(source, class_prefix)
    if kind is None:
        return [], parse_enum_functions(source)

    # Also parse BindCommon if it exists
    common_regs = parse_bind_common(source, class_prefix)

    # Resolve parent name
    parent_name = None
    if parent_const:
        parent_name = resolve_constant(parent_const, name_map)

    # Resolve global name
    global_name = None
    if global_const:
        if global_const in name_map:
            global_name = name_map[global_const]
        else:
            global_name = global_const  # Already a resolved string

    if global_name is None:
        # Fallback: derive from class prefix
        global_name = class_prefix.replace('_Lua', '')

    # Extract function bodies for method analysis
    all_bodies = extract_function_bodies(source)

    # Determine if methods use self (class or value type)
    is_self_type = kind in ("class", "value")

    # Process all registered methods
    all_regs = common_regs + registrations
    methods = []
    seen_method_names = set()

    for lua_name, cpp_func in all_regs:
        # Skip metamethods
        if lua_name in SKIP_METAMETHODS:
            continue

        is_operator = lua_name in OPERATOR_METAMETHODS
        is_alias = lua_name != cpp_func

        # Find the function body
        body_key = f"{class_prefix}::{cpp_func}"
        body = all_bodies.get(body_key)

        params = []
        returns = []

        if body:
            params = parse_check_macros_from_body(body, is_self_type)
            returns = parse_return_types(body)

        method = MethodInfo(
            name=lua_name,
            cpp_func=cpp_func,
            params=params,
            returns=returns,
            is_alias=is_alias
        )

        # Skip duplicate operators (e.g. Add and __add both exist)
        if is_operator:
            continue

        if lua_name not in seen_method_names:
            methods.append(method)
            seen_method_names.add(lua_name)

    # Create TypeInfo
    type_info = TypeInfo(
        lua_name=global_name,
        kind=kind,
        parent=parent_name,
        methods=methods,
        source_file=basename,
        extra_globals=extra_globals
    )

    # Add fields for value types
    if kind == "value":
        if global_name == "Vector":
            type_info.fields = [
                ("x", "number"), ("y", "number"),
                ("z", "number"), ("w", "number"),
            ]
            type_info.operators = [
                ("__add", "Vector + Vector|number -> Vector"),
                ("__sub", "Vector - Vector|number -> Vector"),
                ("__mul", "Vector * Vector|number -> Vector"),
                ("__div", "Vector / Vector|number -> Vector"),
                ("__unm", "-Vector -> Vector"),
                ("__eq", "Vector == Vector -> boolean"),
            ]
        elif global_name == "Rect":
            type_info.fields = [
                ("x", "number"), ("y", "number"),
                ("w", "number"), ("h", "number"),
                ("width", "number"), ("height", "number"),
            ]

    # Also collect enums from this file (e.g. Input_Lua has BindKeyTable etc.)
    enums = parse_enum_functions(source)

    return [type_info], enums


# ---------------------------------------------------------------------------
# Stub generation
# ---------------------------------------------------------------------------

def generate_class_stub(type_info):
    """Generate a .lua stub file for a class type."""
    lines = []
    lines.append(f"--- @meta")
    lines.append(f"")

    # Class annotation
    if type_info.parent:
        lines.append(f"---@class {type_info.lua_name} : {type_info.parent}")
    else:
        lines.append(f"---@class {type_info.lua_name}")

    # Fields (for value types)
    for field_name, field_type in type_info.fields:
        lines.append(f"---@field {field_name} {field_type}")

    # Operators
    for op, desc in type_info.operators:
        lines.append(f"--- {desc}")

    lines.append(f"{type_info.lua_name} = {{}}")
    lines.append(f"")

    # Methods
    for method in type_info.methods:
        lines.extend(_generate_method_lines(type_info.lua_name, method, use_colon=True))
        lines.append(f"")

    return "\n".join(lines)


def generate_module_stub(type_info):
    """Generate a .lua stub file for a module type."""
    lines = []
    lines.append(f"--- @meta")
    lines.append(f"")
    lines.append(f"---@class {type_info.lua_name}Module")
    lines.append(f"{type_info.lua_name} = {{}}")
    lines.append(f"")

    for method in type_info.methods:
        lines.extend(_generate_method_lines(type_info.lua_name, method, use_colon=False))
        lines.append(f"")

    # Extra global shortcuts
    for global_name, cpp_func in type_info.extra_globals:
        lines.append(f"---@type function")
        lines.append(f"{global_name} = {type_info.lua_name}.{cpp_func.split('::')[-1]}")
        lines.append(f"")

    return "\n".join(lines)


def generate_value_stub(type_info):
    """Generate a .lua stub file for a value type (Vector, Rect, etc.)."""
    lines = []
    lines.append(f"--- @meta")
    lines.append(f"")

    # Class annotation
    lines.append(f"---@class {type_info.lua_name}")

    # Fields
    for field_name, field_type in type_info.fields:
        lines.append(f"---@field {field_name} {field_type}")

    # Operators
    for op, desc in type_info.operators:
        lines.append(f"--- {desc}")

    lines.append(f"{type_info.lua_name} = {{}}")
    lines.append(f"")

    for method in type_info.methods:
        # Value types use colon syntax (they have self)
        lines.extend(_generate_method_lines(type_info.lua_name, method, use_colon=True))
        lines.append(f"")

    # Extra global shortcuts
    for global_name, cpp_func in type_info.extra_globals:
        func_name = cpp_func.split('::')[-1]
        # Find the method to copy its annotations
        matching = [m for m in type_info.methods if m.cpp_func == func_name]
        if matching:
            m = matching[0]
            for p in m.params:
                opt = "?" if p.optional else ""
                lines.append(f"---@param {p.name}{opt} {p.lua_type}")
            for ret in m.returns:
                lines.append(f"---@return {ret}")
        lines.append(f"function {global_name}(...) end")
        lines.append(f"")

    return "\n".join(lines)


def generate_enum_stub(enum_info):
    """Generate a .lua stub file for an enum type."""
    lines = []
    lines.append(f"--- @meta")
    lines.append(f"")
    lines.append(f"---@enum {enum_info.global_name}")
    lines.append(f"{enum_info.global_name} = {{")

    for i, entry in enumerate(enum_info.entries):
        comma = "," if i < len(enum_info.entries) - 1 else ""
        lines.append(f"    {entry.name} = 0{comma}")

    lines.append(f"}}")
    lines.append(f"")

    return "\n".join(lines)


def _generate_method_lines(type_name, method, use_colon):
    """Generate annotation lines for a single method."""
    lines = []

    # Parameter annotations
    for param in method.params:
        opt = "?" if param.optional else ""
        lines.append(f"---@param {param.name}{opt} {param.lua_type}")

    # Return annotations
    if method.returns:
        ret_str = ", ".join(method.returns)
        lines.append(f"---@return {ret_str}")

    # Function signature
    sep = ":" if use_colon else "."
    param_names = ", ".join(p.name for p in method.params)
    lines.append(f"function {type_name}{sep}{method.name}({param_names}) end")

    return lines


# ---------------------------------------------------------------------------
# Entry point
# ---------------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(
        description="Generate EmmyLua stubs from Octave C++ Lua bindings"
    )
    parser.add_argument(
        "--input", "-i",
        default="Engine/Source/LuaBindings",
        help="Input directory containing *_Lua.cpp/h files"
    )
    parser.add_argument(
        "--output", "-o",
        default="Engine/Generated/LuaMeta",
        help="Output directory for generated .lua stubs"
    )
    parser.add_argument(
        "--verbose", "-v",
        action="store_true",
        help="Print detailed processing info"
    )
    args = parser.parse_args()

    input_dir = args.input
    output_dir = args.output

    if not os.path.isdir(input_dir):
        print(f"Error: Input directory not found: {input_dir}", file=sys.stderr)
        sys.exit(1)

    os.makedirs(output_dir, exist_ok=True)

    print(f"Input:  {os.path.abspath(input_dir)}")
    print(f"Output: {os.path.abspath(output_dir)}")

    # Phase 1: Parse headers
    print("Parsing headers...")
    name_map, flag_map, _ = parse_headers(input_dir)
    check_map = build_check_map_from_headers(input_dir)

    # Merge into global check map
    ENGINE_CHECK_MAP.update(check_map)

    if args.verbose:
        print(f"  Found {len(name_map)} LUA_NAME constants")
        print(f"  Found {len(flag_map)} LUA_FLAG constants")
        print(f"  Found {len(ENGINE_CHECK_MAP)} engine CHECK_* macros")

    # Phase 2: Process all binding .cpp files
    print("Processing binding files...")
    all_types = []
    all_enums = []

    cpp_files = sorted(f for f in os.listdir(input_dir) if f.endswith('_Lua.cpp'))
    # Also include Misc_Lua.cpp
    if 'Misc_Lua.cpp' not in cpp_files:
        misc_path = os.path.join(input_dir, 'Misc_Lua.cpp')
        if os.path.exists(misc_path):
            cpp_files.append('Misc_Lua.cpp')

    for fname in cpp_files:
        filepath = os.path.join(input_dir, fname)
        types, enums = process_binding_file(filepath, name_map, ENGINE_CHECK_MAP, args.verbose)
        all_types.extend(types)
        all_enums.extend(enums)

    # Deduplicate enums by global name
    seen_enum_names = set()
    unique_enums = []
    for e in all_enums:
        if e.global_name not in seen_enum_names:
            unique_enums.append(e)
            seen_enum_names.add(e.global_name)
    all_enums = unique_enums

    print(f"  Found {len(all_types)} types, {len(all_enums)} enums")

    # Phase 3: Generate stubs
    print("Generating stubs...")
    generated_count = 0

    for type_info in all_types:
        if type_info.kind == "class":
            content = generate_class_stub(type_info)
        elif type_info.kind == "module":
            content = generate_module_stub(type_info)
        elif type_info.kind == "value":
            content = generate_value_stub(type_info)
        else:
            continue

        out_path = os.path.join(output_dir, f"{type_info.lua_name}.lua")
        with open(out_path, "w", encoding="utf-8", newline="\n") as f:
            f.write(content)
        generated_count += 1
        if args.verbose:
            method_count = len(type_info.methods)
            print(f"  {type_info.lua_name}.lua ({type_info.kind}, {method_count} methods)")

    for enum_info in all_enums:
        content = generate_enum_stub(enum_info)
        out_path = os.path.join(output_dir, f"{enum_info.global_name}.lua")
        with open(out_path, "w", encoding="utf-8", newline="\n") as f:
            f.write(content)
        generated_count += 1
        if args.verbose:
            print(f"  {enum_info.global_name}.lua (enum, {len(enum_info.entries)} entries)")

    print(f"Generated {generated_count} stub files in {os.path.abspath(output_dir)}")
    print("Done!")


if __name__ == "__main__":
    main()
