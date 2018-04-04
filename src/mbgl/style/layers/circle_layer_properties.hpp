// This file is generated. Edit scripts/generate-style-code.js, then run `make style-code`.

#pragma once

#include <mbgl/style/types.hpp>
#include <mbgl/style/layout_property.hpp>
#include <mbgl/style/paint_property.hpp>
#include <mbgl/style/properties.hpp>
#include <mbgl/programs/attributes.hpp>
#include <mbgl/programs/uniforms.hpp>

namespace mbgl {
namespace style {

struct CircleRadius : DataDrivenPaintProperty<float, gl::Attributes<attributes::a_radius>, gl::Uniforms<uniforms::u_radius>> {
    static float defaultValue() { return 5; }
};

struct CircleColor : DataDrivenPaintProperty<Color, gl::Attributes<attributes::a_color>, gl::Uniforms<uniforms::u_color>> {
    static Color defaultValue() { return Color::black(); }
};

struct CircleBlur : DataDrivenPaintProperty<float, gl::Attributes<attributes::a_blur>, gl::Uniforms<uniforms::u_blur>> {
    static float defaultValue() { return 0; }
};

struct CircleOpacity : DataDrivenPaintProperty<float, gl::Attributes<attributes::a_opacity>, gl::Uniforms<uniforms::u_opacity>> {
    static float defaultValue() { return 1; }
};

struct CircleTranslate : PaintProperty<std::array<float, 2>> {
    static std::array<float, 2> defaultValue() { return {{ 0, 0 }}; }
};

struct CircleTranslateAnchor : PaintProperty<TranslateAnchorType> {
    static TranslateAnchorType defaultValue() { return TranslateAnchorType::Map; }
};

struct CirclePitchScale : PaintProperty<CirclePitchScaleType> {
    static CirclePitchScaleType defaultValue() { return CirclePitchScaleType::Map; }
};

struct CirclePitchAlignment : PaintProperty<AlignmentType> {
    static AlignmentType defaultValue() { return AlignmentType::Viewport; }
};

struct CircleStrokeWidth : DataDrivenPaintProperty<float, gl::Attributes<attributes::a_stroke_width>, gl::Uniforms<uniforms::u_stroke_width>> {
    static float defaultValue() { return 0; }
};

struct CircleStrokeColor : DataDrivenPaintProperty<Color, gl::Attributes<attributes::a_stroke_color>, gl::Uniforms<uniforms::u_stroke_color>> {
    static Color defaultValue() { return Color::black(); }
};

struct CircleStrokeOpacity : DataDrivenPaintProperty<float, gl::Attributes<attributes::a_stroke_opacity>, gl::Uniforms<uniforms::u_stroke_opacity>> {
    static float defaultValue() { return 1; }
};

class CirclePaintProperties : public Properties<
    CircleRadius,
    CircleColor,
    CircleBlur,
    CircleOpacity,
    CircleTranslate,
    CircleTranslateAnchor,
    CirclePitchScale,
    CirclePitchAlignment,
    CircleStrokeWidth,
    CircleStrokeColor,
    CircleStrokeOpacity
> {};

} // namespace style
} // namespace mbgl
