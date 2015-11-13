#version 400 compatibility
uniform float uKa, uKd, uKs; // coefficients of each type of lighting
uniform vec3 uColor; // object color
uniform vec3 uSpecularColor; // light color
uniform float uShininess;
uniform float uS0, uT0;
uniform float uSize;
uniform sampler2D uTexUnit;
uniform float Ds, Dt;
in vec2 vST; // texture cords
in vec3 vN; // normal vector
in vec3 vL; // vector from point to light
in vec3 vE; // vector from point to eye

// specular exponent
void
main( )
{
	vec3 Normal = normalize(vN);
	vec3 Light = normalize(vL);
	vec3 myColor = uColor;
	vec3 Eye = normalize(vE);
	vec3 ambient;
	//if( Ds+ uS0-uSize/2. <= vST.s && vST.s <= uS0+uSize/2.+Ds && Dt+uT0-uSize/2. <= vST.t && vST.t <= uT0+uSize/2.+Dt )
	if (vST.s >= sin(360*uS0) + Ds-uSize/2 && vST.s < Ds +uSize/2. && vST.t  >= cos(360*uT0) + Dt-uSize/2. && vST.t < cos(360*uT0)+ Dt*2+uSize/2.)
	//if (vST.s >= sin(360.*uS0) + Ds-uSize/2 && vST.s < Ds +uSize/2. && vST.t  >= cos(360*uT0) + Dt+uSize/2. && vST.t < cos(360*uT0)+ Dt*2+uSize/2.)
	
	//if (vST.s >= .25 && vST.s <= .5 && vST.t >=.5 + cos() && vST.t <= 1.)
	//if(vST.t && vST.t <5)
	{
		myColor = vec3( 0., 0., 1. );
	}
	ambient = uKa * myColor;
	float d = max( dot(Normal,Light), 0. ); // only do diffuse if the light can see the point
	vec3 diffuse = uKd * d * uColor;
	float s = 0.;
	if( dot(Normal,Light) > 0. ) // only do specular if the light can see the point
	{
		vec3 ref = normalize( reflect( -Light, Normal ) );
		s = .6*pow( max( dot(Eye,ref),0. ), uShininess );
	}
	vec3 specular = uKs * s * uSpecularColor;
	gl_FragColor = vec4( ambient + diffuse + specular, 1. );
	

	
}