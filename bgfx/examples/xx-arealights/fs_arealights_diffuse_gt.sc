$input v_normal, v_tangent, v_wpos, v_shadowcoord, v_texcoord0

#include "common.sh"
#include "squad.sh"

// -------------------------------------------------------------------------------------------------
void main()
{
    ShadingContext s = InitShadingContext(v_normal, v_tangent, v_wpos, u_viewPosition.xyz, v_texcoord0);

    float alpha = 0.0001;

    mat3 t2w = BasisFrisvad(s.n);
    mat3 w2t = transpose(t2w);

    // express receiver dir in tangent space
    vec3 o = mul(w2t, s.o);

    vec3 ex = u_quadPoints[1].xyz - u_quadPoints[0].xyz;
    vec3 ey = u_quadPoints[3].xyz - u_quadPoints[0].xyz;
    vec2 uvScale = vec2(length(ex), length(ey));
    SphQuad squad = SphQuadInit(u_quadPoints[0].xyz, ex, ey, s.p);

    float rcpSolidAngle = 1.0/squad.S;

    vec3 quadn = normalize(cross(ex, ey));
    quadn = mul(w2t, quadn);

    vec2 jitter = FAST_32_hash(gl_FragCoord.xy).xy;

    // integrate
    vec3 Lo_i = vec3(0, 0, 0);

#if BGFX_SHADER_LANGUAGE_HLSL != 3
    for (int t = 0; t < NUM_SAMPLES; t++)
    {
        float u1 = fract(jitter.x + u_samples[t].x);
        float u2 = fract(jitter.y + u_samples[t].y);

        // light sample
        {
            vec3 lightPos = SphQuadSample(squad, u1, u2);

            vec3 i = normalize(lightPos - s.p);
            i = mul(w2t, i);

            float cos_theta_i = i.z;

            // Derive UVs from sample point
            vec3 pd = lightPos - u_quadPoints[0].xyz;
            vec2 uv = vec2(dot(pd, squad.x), dot(pd, squad.y))/uvScale;

            vec3 color = texture2DLod(s_texColorMap, uv, 0.0).rgb;

            float pdfBRDF = 1.0/(2.0*M_PI);
            vec3 fr_p = color/M_PI;

            float pdfLight = rcpSolidAngle;

            if (cos_theta_i > 0.0 && (dot(i, quadn) < 0.0 || u_twoSided))
                Lo_i += fr_p*cos_theta_i/(pdfBRDF + pdfLight);
        }

        // BRDF sample
        {
            float phi = 2.0*M_PI*u1;
            float cp = cos(phi);
            float sp = sin(phi);

            float r = sqrt(u2);
            vec3 i = vec3(r*cp, r*sp, sqrt(1.0 - r*r));

            float cos_theta_i = i.z;

            vec2 uv = vec2(0, 0);
            bool hit = QuadRayTest(u_quadPoints, s.p, mul(t2w, i), uv, u_twoSided);

            vec3 color = texture2DLod(s_texColorMap, uv, 0.0).rgb;
            color = hit ? color : vec3(0, 0, 0);

            float pdfBRDF = cos_theta_i/M_PI;
            vec3 fr_p = color/M_PI;

            float pdfLight = hit ? rcpSolidAngle : 0.0;

            if (cos_theta_i > 0.0 && pdfBRDF > 0.0)
                Lo_i += fr_p*cos_theta_i/(pdfBRDF + pdfLight);
        }
    }
#endif

    // scale by light intensity
    Lo_i *= u_lightIntensity;

    // scale by diffuse albedo
    Lo_i *= s.diffColor*u_albedo.rgb;

    // normalize
    Lo_i /= float(NUM_SAMPLES);

    // set output
    gl_FragColor = vec4(Lo_i, 1.0);
}
