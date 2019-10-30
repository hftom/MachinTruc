#include <movit/blur_effect.h>

#include "glmask.h"



GLMask::GLMask(QString id, QString name ) : GLFilter( id, name )
{
	blur = NULL;
}



void GLMask::setParameters()
{
	selectionMode = addParameter( "selectionMode", "Selection:", Parameter::PGROUPCOMBO, 0, 0, 0, false );

	Parameter *item = addParameter( "colorGroup", "Color", Parameter::PGROUPITEM, 0, 0, 0, false );
	Q_UNUSED(item);
	hsvColor = addParameter( "color", tr("Color:"), Parameter::PCOLORWHEEL, QColor::fromRgbF( 0, 0.5, 0.5 ), QColor::fromRgbF( 0, 0, 0 ), QColor::fromRgbF( 1, 1, 1 ), false );
	hsvColor->layout.setLayout( 100, 0, 3, 1 );
	varianceH = addParameter( "varianceH", tr("H variance:"), Parameter::PDOUBLE, 25.0, 0.0, 180.0, false );
	varianceH->layout.setLayout( 100, 1);
	varianceS = addParameter( "varianceS", tr("S variance:"), Parameter::PDOUBLE, 0.4, 0.0, 0.5, false );
	varianceS->layout.setLayout( 101, 1);
	varianceV = addParameter( "varianceV", tr("V variance:"), Parameter::PDOUBLE, 0.4, 0.0, 0.5, false );
	varianceV->layout.setLayout( 102, 1 );
	gain = addParameter( "gain", tr("Gain:"), Parameter::PDOUBLE, 1.0, 1.0, 20.0, false );
	gain->layout.setLayout( 103, 0, 1, 2 );
	smoothColor = addParameter("smoothColor", tr("Smooth selection"), Parameter::PDOUBLE, 0.0, 0.0, 1.0, false );
	smoothColor->layout.setLayout( 104, 0, 1, 2 );
	invertColor = addBooleanParameter( "invert", tr("Invert selection"), 0 );
	invertColor->layout.setLayout( 105, 0, 1, 2);
	showColor = addBooleanParameter( "showColor", tr("Show selection"), 0 );
	showColor->layout.setLayout( 106, 0, 1, 2 );
}



QString GLMask::getMaskDescriptor( double pts, Frame *src, Profile *p  )
{
	Q_UNUSED(pts);
	Q_UNUSED(src);
	Q_UNUSED(p);
	switch (getParamValue(selectionMode).toInt()) {
		case 1: return QString("color %1 %2 %3 %4").arg(getParamValue(invertColor).toInt())
					.arg(getParamValue(showColor).toInt())
					.arg(getParamValue(smoothColor).toDouble() > 0 ? 1 : 0)
					// We do access blur and mask in processMask, and they are created in setGraph, that is, at movit chain finalize.
					// So, make sure the chain is rebuilt by printing our pointer in the fingerprint.
					.arg(QString().sprintf("%p", this));
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
			if (blur) {
				blur->set_float("radius", getParamValue( smoothColor ).toDouble() * 4.0) && true;
			}
			return mask->set_vec3( "hsvColor", (float*)&col )
				&& mask->set_float("varianceH", qMax(getParamValue( varianceH ).toDouble(), 0.0001))
				&& mask->set_float("varianceS", qMax(getParamValue( varianceS ).toDouble(), 0.0001))
				&& mask->set_float("varianceV", qMax(getParamValue( varianceV ).toDouble(), 0.0001))
				&& mask->set_float("gain", getParamValue( gain ).toDouble());
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

			mix->set_int("invert", getParamValue(invertColor).toInt())
			&& mix->set_int("show", getParamValue(showColor).toInt());

			Node *mask_node = graph->add_node(mask);
			Node *mix_node = graph->add_node(mix);
			if (getParamValue(smoothColor).toDouble() > 0) {
				blur = new BlurEffect;
				blur->set_int( "num_taps", 6 ) && true;
				Node *blur_node = graph->add_node(blur);
				graph->replace_receiver(receiverSender, blur_node);
				graph->connect_nodes(blur_node, mask_node);
			}
			else {
				blur = NULL;
				graph->replace_receiver(receiverSender, mask_node);
			}

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
"vec4 FUNCNAME(vec2 tc) {\n"
"	vec3 hsv = PREFIX(rgb2hsv)(INPUT(tc).rgb, PREFIX(hsvColor).x + 180.0);\n"
"	float offset = 180.0 - PREFIX(hsvColor).x;\n"
"	hsv.x = mod(hsv.x + offset, 360.0);\n"
"	float hx = PREFIX(hsvColor).x + offset;\n"
"\n"
"	float d = distance(hsv.x, hx) / PREFIX(varianceH);\n"
"	d += distance(hsv.y, PREFIX(hsvColor).y) / PREFIX(varianceS);\n"
"	d += distance(hsv.z, PREFIX(hsvColor).z) / PREFIX(varianceV);\n"
"	d = min(d / 3.0, 1.0);\n"
"	if (d <= 0.5) {\n"
"		d = pow(2 * d, PREFIX(gain)) / 2.0;\n"
"	}\n"
"	else {\n"
"		d = -(pow(2 * (1.0 - d), PREFIX(gain)) / 2.0) + 1.0;\n"
"	}\n"
"	return vec4(1.0 - d);\n"
"}\n";



MaskEffect::MaskEffect() : hsvColor(0.0f, 0.0f, 0.5f),
	varianceH(0),
	varianceS(0),
	varianceV(0),
	gain(0)
{
	register_vec3("hsvColor", (float*)&hsvColor);
	register_float("varianceH", &varianceH);
	register_float("varianceS", &varianceS);
	register_float("varianceV", &varianceV);
	register_float("gain", &gain);
}



std::string MaskEffect::effect_type_id() const
{
	return "MaskEffect";
}



std::string MaskEffect::output_fragment_shader()
{
	QString s = GLFilter::getShaderRGBTOHSV();
	s.append(mask_shader);
	return s.toLatin1().data();
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
"	return vec4(1.0) * mask + vec4(0.0, 0.0, 0.0, 1.0 - mask);\n"
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
