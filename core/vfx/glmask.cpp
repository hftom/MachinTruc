#include "glmask.h"



GLMask::GLMask(QString id, QString name ) : GLFilter( id, name )
{
}



void GLMask::setParameters()
{
	selectionMode = addParameter( "selectionMode", "Selection:", Parameter::PGROUPCOMBO, 0, 0, 0, false );

	Parameter *item = addParameter( "colorGroup", "Color", Parameter::PGROUPITEM, 0, 0, 0, false );
	hsvColor = addParameter( "color", tr("Color:"), Parameter::PCOLORWHEEL, QColor::fromRgbF( 0, 0.5, 0.5 ), QColor::fromRgbF( 0, 0, 0 ), QColor::fromRgbF( 1, 1, 1 ), false );
	hsvColor->layout.setLayout( 100, 0, 3, 1 );
	varianceH = addParameter( "varianceH", tr("H variance:"), Parameter::PDOUBLE, 25.0, 0.0, 180.0, false );
	varianceH->layout.setLayout( 100, 1);
	varianceS = addParameter( "varianceS", tr("S variance:"), Parameter::PDOUBLE, 0.5, 0.0, 0.5, false );
	varianceS->layout.setLayout( 101, 1);
	varianceV = addParameter( "varianceV", tr("V variance:"), Parameter::PDOUBLE, 0.5, 0.0, 0.5, false );
	varianceV->layout.setLayout( 102, 1 );
	smoothSelect = addParameter( "smoothSelect", tr("Smoothing"), Parameter::PDOUBLE, 1.0, 0.0, 1.0, false);
	smoothSelect->layout.setLayout( 103, 0, 1, 2 );
	invertColor = addBooleanParameter( "invert", tr("Invert selection"), 0 );
	invertColor->layout.setLayout( 104, 0, 1, 2);
	showColor = addBooleanParameter( "showColor", tr("Show selection"), 0 );
	showColor->layout.setLayout( 105, 0, 1, 2 );
}



QString GLMask::getMaskDescriptor( double pts, Frame *src, Profile *p  )
{
	Q_UNUSED(pts);
	Q_UNUSED(src);
	Q_UNUSED(p);
	switch (getParamValue(selectionMode).toInt()) {
		case 1: return QString("color %1 %2").arg(getParamValue(invertColor).toInt()).arg(getParamValue(showColor).toInt());
		default: return "";
	}
}



bool GLMask::processMask( double pts, Frame *src, Profile *p )
{
	Q_UNUSED(pts);
	Q_UNUSED(src);
	Q_UNUSED(p);
	switch (getParamValue(selectionMode).toInt()) {
		case 1: {
			QColor c = getParamValue( hsvColor ).value<QColor>();
			RGBTriplet col = RGBTriplet( 360.0 * c.redF(), c.greenF(), c.blueF() );
			return mask->set_vec3( "hsvColor", (float*)&col )
				&& mask->set_float("varianceH", getParamValue( varianceH ).toDouble())
				&& mask->set_float("varianceS", getParamValue( varianceS ).toDouble())
				&& mask->set_float("varianceV", getParamValue( varianceV ).toDouble())
				&& mask->set_float("smoothSelect", getParamValue( smoothSelect ).toDouble());
		}
		default: return true;
	}
}



void GLMask::setGraph(EffectChain *graph, Node *input, Node *receiverSender, Node *effect)
{
	switch (getParamValue(selectionMode).toInt()) {
		case 1: {
			mask = new MaskEffect();
			mix = new MixMaskEffect();

			mix->set_int("invert", getParamValue(invertColor).toInt());
			mix->set_int("show", getParamValue(showColor).toInt());

			Node *mask_node = graph->add_node(mask);
			Node *mix_node = graph->add_node(mix);
			graph->replace_receiver(receiverSender, mask_node);

			graph->connect_nodes(input, effect);

			graph->connect_nodes(input, mix_node);
			graph->connect_nodes(effect, mix_node);
			graph->connect_nodes(mask_node, mix_node);

			graph->replace_sender(receiverSender, mix_node);

			break;
		}
		default: {
			graph->replace_receiver(receiverSender, effect);
			graph->replace_sender(receiverSender, effect);
		}
	}
}



static const char* mask_shader =
"vec3 PREFIX(rgb2hsv)(vec3 rgb) {\n"
"	vec3 hsv;\n"
"	float min = rgb.r < rgb.g ? rgb.r : rgb.g;\n"
"	min = min < rgb.b ? min : rgb.b;\n"
"	float max = rgb.r > rgb.g ? rgb.r : rgb.g;\n"
"	max = max > rgb.b ? max : rgb.b;\n"
"	hsv.z = max;\n"
"	float delta = max - min;\n"
"	if (delta <= 0.0 || max <= 0.0) {\n"
"		hsv.y = 0.0;\n"
"		hsv.x = PREFIX(hsvColor).x;\n"
"		return hsv;\n"
"	}\n"
"	hsv.y = (delta / max);\n"
"	if ( rgb.r == max )\n"
"		hsv.x = ( rgb.g - rgb.b ) / delta;\n"
"	else if ( rgb.g >= max )\n"
"		hsv.x = 2.0 + ( rgb.b - rgb.r ) / delta;\n"
"	else\n"
"		hsv.x = 4.0 + ( rgb.r - rgb.g ) / delta;\n"
"\n"
"	hsv.x *= 60.0;\n"
"	if ( hsv.x < 0.0 )\n"
"		hsv.x += 360.0;\n"
"\n"
"	return hsv;\n"
"}\n"
"\n"
"vec4 FUNCNAME(vec2 tc) {\n"
"	vec3 hsv = PREFIX(rgb2hsv)(INPUT(tc).rgb);\n"
"	float offset = 180.0 - PREFIX(hsvColor).x;\n"
"	hsv.x = mod(hsv.x + offset, 360.0);\n"
"	float hx = PREFIX(hsvColor).x + offset;\n"
"	if ( distance(hsv.x, hx) <= PREFIX(varianceH)\n"
"		&& distance(hsv.y, PREFIX(hsvColor).y) <= PREFIX(varianceS)\n"
"		&& distance(hsv.z, PREFIX(hsvColor).z) <= PREFIX(varianceV) )\n"
"	{\n"
"		float d = distance(hsv.x, hx) / PREFIX(varianceH);\n"
"		d += distance(hsv.y, PREFIX(hsvColor).y) / PREFIX(varianceS);\n"
"		d += distance(hsv.z, PREFIX(hsvColor).z) / PREFIX(varianceV);\n"
"		d /= 3.0;\n"
"		return vec4(1.0 - d * PREFIX(smoothSelect));\n"
"	}\n"
"\n"
"	return vec4(0);\n"
"}\n";



MaskEffect::MaskEffect() : hsvColor(0.0f, 0.0f, 0.5f),
	varianceH(0),
	varianceS(0),
	varianceV(0),
	smoothSelect(1)
{
	register_vec3("hsvColor", (float*)&hsvColor);
	register_float("varianceH", &varianceH);
	register_float("varianceS", &varianceS);
	register_float("varianceV", &varianceV);
	register_float("smoothSelect", &smoothSelect);
}



std::string MaskEffect::effect_type_id() const
{
	return "MaskEffect";
}



std::string MaskEffect::output_fragment_shader()
{
	return mask_shader;
}



static const char* mix_mask_shader =
"vec4 FUNCNAME(vec2 tc) {\n"
"	vec4 org = INPUT1(tc);\n"
"	vec4 effect = INPUT2(tc);\n"
"#if INVERT\n"
"	float mask = 1.0 - INPUT3(tc).a;\n"
"#else\n"
"	float mask = INPUT3(tc).a;\n"
"#endif\n"
"\n"
"#if SHOW\n"
"	return vec4(1.0, 0.25, 0.25, 1.0) * 0.8 * mask + (1.0 - (0.8 * mask)) * org;\n"
"#else\n"
"	return vec4(mix( org, effect, mask ));\n"
"#endif\n"
"#undef INVERT\n"
"#undef SHOW\n"
"}\n";



MixMaskEffect::MixMaskEffect() : invert(0), show(0)
{
	register_int("invert", &invert);
	register_int("show", &show);
}



std::string MixMaskEffect::effect_type_id() const
{
	return "MixMaskEffect";
}



std::string MixMaskEffect::output_fragment_shader()
{
	QString s = mix_mask_shader;
	if ( invert )
		s.prepend( "#define INVERT 1\n" );
	else
		s.prepend( "#define INVERT 0\n" );
	if ( show )
		s.prepend( "#define SHOW 1\n" );
	else
		s.prepend( "#define SHOW 0\n" );

	return s.toLatin1().data();
}
