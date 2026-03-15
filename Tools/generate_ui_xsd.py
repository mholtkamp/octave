#!/usr/bin/env python3
"""
Generate an XSD schema for Octave Engine UIDocument XML files.

Produces an XSD that XML editors (VS Code + Red Hat XML, IntelliJ, etc.)
can use for autocomplete, validation, and documentation of .xml UI files.

Usage:
    python Tools/generate_ui_xsd.py
    python Tools/generate_ui_xsd.py --output Engine/Generated/XML/OctaveUIDocument.xsd --verbose
"""

import argparse
import os
import sys
import xml.etree.ElementTree as ET
from xml.dom import minidom

# ---------------------------------------------------------------------------
# XSD namespace
# ---------------------------------------------------------------------------

XS = "http://www.w3.org/2001/XMLSchema"

# ---------------------------------------------------------------------------
# Enum values (from UITypes.cpp and UILoader.cpp)
# ---------------------------------------------------------------------------

ANCHOR_VALUES = [
    "top-left", "top-center", "top-right",
    "center-left", "center", "center-right",
    "bottom-left", "bottom-center", "bottom-right",
    "top-stretch", "bottom-stretch",
    "left-stretch", "right-stretch",
    "center-h-stretch", "center-v-stretch",
    "full-stretch",
]

BOOLEAN_VALUES = ["true", "false", "1", "0"]

WORD_WRAP_VALUES = ["true", "false", "1", "0", "break-word"]

FLEX_DIRECTION_VALUES = ["row", "column"]

# ---------------------------------------------------------------------------
# Attribute definitions
# ---------------------------------------------------------------------------

# Common attributes applied to all widget elements
COMMON_ATTRIBUTES = [
    ("id",               "xs:string",    False),
    ("class",            "xs:string",    False),
    ("style",            "xs:string",    False),
    ("name",             "xs:string",    False),
    ("visible",          "booleanType",  False),
    ("active",           "booleanType",  False),
    ("width",            "xs:string",    False),
    ("height",           "xs:string",    False),
    ("anchor",           "anchorType",   False),
    ("opacity",          "xs:string",    False),
    ("color",            "xs:string",    False),
    ("background-color", "xs:string",    False),
    ("bg-color",         "xs:string",    False),
]

# Event attributes applied to all widget elements
EVENT_ATTRIBUTES = [
    ("on-click",   "xs:string", False),
    ("on-hover",   "xs:string", False),
    ("on-press",   "xs:string", False),
    ("on-release", "xs:string", False),
]

# Extra attributes per element (name, type, required)
ELEMENT_SPECIFIC_ATTRS = {
    "flex":        [("direction", "flexDirectionType", False), ("spacing", "xs:string", False)],
    "ArrayWidget": [("direction", "flexDirectionType", False), ("spacing", "xs:string", False)],
    "button":      [("src", "xs:string", False), ("text", "xs:string", False),
                    ("font", "xs:string", False), ("font-size", "xs:string", False)],
    "Button":      [("src", "xs:string", False), ("text", "xs:string", False),
                    ("font", "xs:string", False), ("font-size", "xs:string", False)],
    "img":         [("src", "xs:string", False)],
    "Quad":        [("src", "xs:string", False)],
    "text":        [("text", "xs:string", False), ("font", "xs:string", False),
                    ("font-size", "xs:string", False), ("word-wrap", "wordWrapType", False)],
    "p":           [("text", "xs:string", False), ("font", "xs:string", False),
                    ("font-size", "xs:string", False), ("word-wrap", "wordWrapType", False)],
    "span":        [("text", "xs:string", False), ("font", "xs:string", False),
                    ("font-size", "xs:string", False), ("word-wrap", "wordWrapType", False)],
    "Text":        [("text", "xs:string", False), ("font", "xs:string", False),
                    ("font-size", "xs:string", False), ("word-wrap", "wordWrapType", False)],
}

# ---------------------------------------------------------------------------
# Element definitions
# ---------------------------------------------------------------------------

# (element_name, category, has_children, mixed_content)
# Categories: "container", "leaf", "text_elem", "meta"
ELEMENTS = [
    # Root
    ("ui",          "container",  True,  False),
    # Containers
    ("div",         "container",  True,  False),
    ("canvas",      "container",  True,  False),
    ("Canvas",      "container",  True,  False),
    ("flex",        "container",  True,  False),
    ("ArrayWidget", "container",  True,  False),
    # Buttons (container + mixed text)
    ("button",      "container",  True,  True),
    ("Button",      "container",  True,  True),
    # Images
    ("img",         "leaf",       False, False),
    ("Quad",        "leaf",       False, False),
    # Text
    ("text",        "text_elem",  False, True),
    ("p",           "text_elem",  False, True),
    ("span",        "text_elem",  False, True),
    ("Text",        "text_elem",  False, True),
    # Poly
    ("poly",        "leaf",       False, False),
    ("polyrect",    "leaf",       False, False),
    ("Poly",        "leaf",       False, False),
    ("PolyRect",    "leaf",       False, False),
]

# Meta elements (no common/event attrs)
META_ELEMENTS = [
    ("link",    [("href", "xs:string", True)]),
    ("style",   []),
    ("include", [("src", "xs:string", True)]),
]


# ---------------------------------------------------------------------------
# XSD builder
# ---------------------------------------------------------------------------

def build_xsd():
    """Build the complete XSD as an ElementTree."""
    schema = ET.Element("xs:schema", {
        "xmlns:xs": XS,
        "elementFormDefault": "qualified",
    })

    # --- Simple types (enums) ---
    _add_enum_type(schema, "anchorType", ANCHOR_VALUES,
                   "Anchor/layout mode for widget positioning")
    _add_enum_type(schema, "booleanType", BOOLEAN_VALUES,
                   "Boolean value (true/false/1/0)")
    _add_enum_type(schema, "wordWrapType", WORD_WRAP_VALUES,
                   "Word wrap mode for text widgets")
    _add_enum_type(schema, "flexDirectionType", FLEX_DIRECTION_VALUES,
                   "Flex layout direction")

    # --- Attribute groups ---
    _add_attribute_group(schema, "commonAttributes", COMMON_ATTRIBUTES)
    _add_attribute_group(schema, "eventAttributes", EVENT_ATTRIBUTES)

    # --- Widget elements group (for recursive child content) ---
    group = ET.SubElement(schema, "xs:group", {"name": "widgetElements"})
    choice = ET.SubElement(group, "xs:choice")
    for name, category, _, _ in ELEMENTS:
        if name != "ui":  # ui is root only
            ET.SubElement(choice, "xs:element", {"ref": name})
    for name, _ in META_ELEMENTS:
        ET.SubElement(choice, "xs:element", {"ref": name})

    # --- Widget element declarations ---
    for name, category, has_children, mixed in ELEMENTS:
        _add_widget_element(schema, name, category, has_children, mixed)

    # --- Meta element declarations ---
    for name, attrs in META_ELEMENTS:
        _add_meta_element(schema, name, attrs)

    # --- Root <ui> element is already declared above; just register it ---
    # (It's both a global element declaration and referenced as the document root)

    return schema


def _add_enum_type(parent, type_name, values, doc=None):
    """Add an xs:simpleType with xs:restriction/xs:enumeration."""
    simple = ET.SubElement(parent, "xs:simpleType", {"name": type_name})
    if doc:
        annot = ET.SubElement(simple, "xs:annotation")
        ET.SubElement(annot, "xs:documentation").text = doc
    restriction = ET.SubElement(simple, "xs:restriction", {"base": "xs:string"})
    for v in values:
        ET.SubElement(restriction, "xs:enumeration", {"value": v})


def _add_attribute_group(parent, group_name, attrs):
    """Add an xs:attributeGroup."""
    ag = ET.SubElement(parent, "xs:attributeGroup", {"name": group_name})
    for attr_name, attr_type, required in attrs:
        attrib = {"name": attr_name, "type": attr_type}
        if required:
            attrib["use"] = "required"
        ET.SubElement(ag, "xs:attribute", attrib)


def _add_widget_element(parent, name, category, has_children, mixed):
    """Add an xs:element for a widget element."""
    elem = ET.SubElement(parent, "xs:element", {"name": name})
    complex_type = ET.SubElement(elem, "xs:complexType")
    if mixed:
        complex_type.set("mixed", "true")

    if has_children:
        seq = ET.SubElement(complex_type, "xs:sequence", {
            "minOccurs": "0",
            "maxOccurs": "unbounded",
        })
        ET.SubElement(seq, "xs:group", {"ref": "widgetElements"})

    # Common + event attribute groups
    ET.SubElement(complex_type, "xs:attributeGroup", {"ref": "commonAttributes"})
    ET.SubElement(complex_type, "xs:attributeGroup", {"ref": "eventAttributes"})

    # Element-specific attributes
    specific = ELEMENT_SPECIFIC_ATTRS.get(name, [])
    for attr_name, attr_type, required in specific:
        attrib = {"name": attr_name, "type": attr_type}
        if required:
            attrib["use"] = "required"
        ET.SubElement(complex_type, "xs:attribute", attrib)


def _add_meta_element(parent, name, attrs):
    """Add an xs:element for a meta element (link, style, include)."""
    elem = ET.SubElement(parent, "xs:element", {"name": name})
    complex_type = ET.SubElement(elem, "xs:complexType")

    # <style> has mixed content (CSS text inside)
    if name == "style":
        complex_type.set("mixed", "true")

    for attr_name, attr_type, required in attrs:
        attrib = {"name": attr_name, "type": attr_type}
        if required:
            attrib["use"] = "required"
        ET.SubElement(complex_type, "xs:attribute", attrib)


# ---------------------------------------------------------------------------
# Output formatting
# ---------------------------------------------------------------------------

def format_xsd(root):
    """Return a pretty-printed XML string for the schema."""
    rough = ET.tostring(root, encoding="unicode")
    dom = minidom.parseString(rough)
    pretty = dom.toprettyxml(indent="  ", encoding=None)
    # minidom adds an xml declaration; keep it but fix encoding attr
    lines = pretty.split("\n")
    # Replace the declaration with a standard one
    if lines and lines[0].startswith("<?xml"):
        lines[0] = '<?xml version="1.0" encoding="UTF-8"?>'
    # Remove blank lines that minidom sometimes inserts
    cleaned = "\n".join(line for line in lines if line.strip())
    return cleaned + "\n"


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(
        description="Generate XSD schema for Octave UIDocument XML files."
    )
    parser.add_argument(
        "--output", "-o",
        default=os.path.join("Engine", "Generated", "XML", "OctaveUIDocument.xsd"),
        help="Output XSD file path (default: Engine/Generated/XML/OctaveUIDocument.xsd)",
    )
    parser.add_argument(
        "--verbose", "-v",
        action="store_true",
        help="Print generation details",
    )
    args = parser.parse_args()

    # Build the schema
    schema = build_xsd()
    xsd_text = format_xsd(schema)

    # Ensure output directory exists
    out_dir = os.path.dirname(args.output)
    if out_dir:
        os.makedirs(out_dir, exist_ok=True)

    # Write
    with open(args.output, "w", encoding="utf-8", newline="\n") as f:
        f.write(xsd_text)

    if args.verbose:
        widget_count = len(ELEMENTS)
        meta_count = len(META_ELEMENTS)
        enum_count = 4  # anchorType, booleanType, wordWrapType, flexDirectionType
        common_attr_count = len(COMMON_ATTRIBUTES)
        event_attr_count = len(EVENT_ATTRIBUTES)
        print(f"Generated {args.output}")
        print(f"  Elements:   {widget_count} widget + {meta_count} meta = {widget_count + meta_count} total")
        print(f"  Enums:      {enum_count} simple types")
        print(f"  Attributes: {common_attr_count} common + {event_attr_count} event")
        unique_specific = set()
        for attrs in ELEMENT_SPECIFIC_ATTRS.values():
            for a, _, _ in attrs:
                unique_specific.add(a)
        print(f"  Specific:   {len(unique_specific)} unique element-specific attributes")
    else:
        print(f"Wrote {args.output}")


if __name__ == "__main__":
    main()
