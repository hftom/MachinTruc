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
	invert = addBooleanParameter( "invert", tr("Invert selection"), 0 );
	invert->layout.setLayout( 103, 0 );
}



QString GLMask::getMaskDescriptor( double pts, Frame *src, Profile *p  )
{
	switch (getParamValue(selectionMode).toInt()) {
		case 1: return "color";
		default: return "";
	}
}



bool GLMask::processMask( double pts, Frame *src, Profile *p )
{
	switch (getParamValue(selectionMode).toInt()) {
		case 1: {
			QColor c = getParamValue( hsvColor ).value<QColor>();
			RGBTriplet col = RGBTriplet( 360.0 * c.redF(), c.greenF(), c.blueF() );
			return mask->set_vec3( "hsvColor", (float*)&col )
				&& mask->set_float("varianceH", getParamValue( varianceH ).toDouble())
				&& mask->set_float("varianceS", getParamValue( varianceS ).toDouble())
				&& mask->set_float("varianceV", getParamValue( varianceV ).toDouble())
				&& mix->set_float("invert", getParamValue( invert ).toInt());
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
