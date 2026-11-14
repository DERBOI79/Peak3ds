#include <3ds.h>
#include <citro3d.h>
#include <stdio.h>
#include <math.h>
#include "vshader_shbin.h"

#define CLEAR_COLOR 0x83C9E8FF
#define DISPLAY_TRANSFER_FLAGS (GX_TRANSFER_FLIP_VERT(0) | GX_TRANSFER_OUT_TILED(0) | GX_TRANSFER_RAW_COPY(0) | GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8) | GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGB8) | GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO))

typedef struct { float x, y, z; } Vec3;
static DVLB_s *shaderDvlb;
static shaderProgram_s shaderProgram;
static int uProjection, uModelView;
static C3D_Mtx projection, modelView;
static C3D_RenderTarget *topTarget;
static PrintConsole bottomConsole;
static float climbHeight = 0.0f;
static float stamina = 100.0f;
static int leftGrip = 1, rightGrip = 1;

static void vertex(float x, float y, float z, float r, float g, float b) {
    C3D_ImmSendAttrib(x, y, z, 1.0f);
    C3D_ImmSendAttrib(r, g, b, 1.0f);
}
static void tri(Vec3 a, Vec3 b, Vec3 c, float r, float g, float bl) {
    vertex(a.x,a.y,a.z,r,g,bl); vertex(b.x,b.y,b.z,r,g,bl); vertex(c.x,c.y,c.z,r,g,bl);
}
static void quad(Vec3 a, Vec3 b, Vec3 c, Vec3 d, float r, float g, float bl) {
    tri(a,b,c,r,g,bl); tri(a,c,d,r*0.92f,g*0.92f,bl*0.92f);
}
static void drawMountain(void) {
    C3D_ImmDrawBegin(GPU_TRIANGLES);
    /* distant layered low-poly peaks */
    tri((Vec3){-10,0,-13},(Vec3){-5,8,-16},(Vec3){0,0,-14},0.40f,0.58f,0.63f);
    tri((Vec3){-2,0,-17},(Vec3){4,10,-19},(Vec3){9,0,-16},0.48f,0.62f,0.66f);
    tri((Vec3){4,0,-19},(Vec3){8,7,-21},(Vec3){13,0,-18},0.38f,0.54f,0.61f);
    /* main cliff: faceted brown/grey rock face */
    tri((Vec3){-5,-1,-4},(Vec3){-2,5,-7},(Vec3){1,0,-5},0.48f,0.39f,0.30f);
    tri((Vec3){-5,-1,-4},(Vec3){-2,5,-7},(Vec3){-6,4,-8},0.58f,0.48f,0.36f);
    tri((Vec3){-2,5,-7},(Vec3){2,7,-9},(Vec3){1,0,-5},0.66f,0.55f,0.42f);
    tri((Vec3){1,0,-5},(Vec3){2,7,-9},(Vec3){5,3,-7},0.43f,0.36f,0.29f);
    tri((Vec3){2,7,-9},(Vec3){6,8,-11},(Vec3){5,3,-7},0.57f,0.48f,0.38f);
    tri((Vec3){5,3,-7},(Vec3){6,8,-11},(Vec3){9,1,-8},0.42f,0.36f,0.30f);
    tri((Vec3){-6,4,-8},(Vec3){-2,5,-7},(Vec3){-4,8,-11},0.70f,0.60f,0.46f);
    tri((Vec3){-4,8,-11},(Vec3){2,7,-9},(Vec3){0,11,-13},0.76f,0.69f,0.56f);
    tri((Vec3){2,7,-9},(Vec3){6,8,-11},(Vec3){0,11,-13},0.66f,0.61f,0.52f);
    /* grassy foothill */
    quad((Vec3){-12,-1,-2},(Vec3){-5,-1,-4},(Vec3){-6,0,-8},(Vec3){-12,0,-7},0.25f,0.52f,0.27f);
    quad((Vec3){5,-1,-3},(Vec3){12,-1,-2},(Vec3){12,0,-8},(Vec3){6,0,-8},0.30f,0.56f,0.28f);
    /* stylized pine trees, made from stacked triangular foliage */
    for (int i=0;i<5;i++) {
        float x=-8.0f+(float)i*3.8f; float z=-5.0f-(float)(i%2)*2.0f;
        tri((Vec3){x-0.8f,0,z},(Vec3){x,2.5f,z},(Vec3){x+0.8f,0,z},0.12f,0.38f,0.22f);
        tri((Vec3){x-0.65f,0.9f,z-0.02f},(Vec3){x,3.3f,z-0.02f},(Vec3){x+0.65f,0.9f,z-0.02f},0.18f,0.48f,0.27f);
        tri((Vec3){x-0.45f,1.7f,z-0.04f},(Vec3){x,4.0f,z-0.04f},(Vec3){x+0.45f,1.7f,z-0.04f},0.22f,0.55f,0.31f);
    }
    C3D_ImmDrawEnd();
}
static void drawHands(void) {
    /* Small warm-colored polygon hands at the bottom edge of the first-person view. */
    C3D_ImmDrawBegin(GPU_TRIANGLES);
    tri((Vec3){-1.25f,-0.85f,-2.0f},(Vec3){-0.95f,-0.55f,-2.0f},(Vec3){-0.70f,-0.88f,-2.0f},0.88f,0.62f,0.43f);
    tri((Vec3){0.70f,-0.88f,-2.0f},(Vec3){0.95f,-0.55f,-2.0f},(Vec3){1.25f,-0.85f,-2.0f},0.88f,0.62f,0.43f);
    C3D_ImmDrawEnd();
}
static void updateBottom(void) {
    consoleSelect(&bottomConsole);
    consoleClear();
    printf("SUMMIT: THE LOST EXPEDITION\n");
    printf("------------------------------\n");
    printf(" EXPEDITION MAP  [FOOTHILLS]\n\n");
    printf("          /\\       SUMMIT\n");
    printf("         /  \\      [  ]\n");
    printf("        / ^^ \\     ALPINE\n");
    printf("       /      \\    [  ]\n");
    printf("      /  YOU   \\   FOOTHILLS\n");
    printf("     /__________\\  [ X ]\n\n");
    printf(" CLIMB HEIGHT: %3d m\n", (int)(climbHeight*12.0f));
    printf(" STAMINA:      %3d%%\n", (int)stamina);
    printf(" LEFT GRIP:  %s\n", leftGrip ? "HELD" : "FREE");
    printf(" RIGHT GRIP: %s\n\n", rightGrip ? "HELD" : "FREE");
    printf(" TOUCH CONTROLS\n");
    printf(" [LEFT]       [RIGHT]\n");
    printf(" GRIP         GRIP\n");
    printf("\n UP: CLIMB   DOWN: REST\n");
    printf(" LEFT/RIGHT: TRAVERSE\n");
    printf(" A: GRAB LEFT   B: GRAB RIGHT\n");
    printf(" X: PULL UP     Y: REST\n");
    printf(" START: EXIT\n");
}
static void sceneInit(void) {
    shaderDvlb = DVLB_ParseFile((u32*)vshader_shbin, vshader_shbin_size);
    shaderProgramInit(&shaderProgram);
    shaderProgramSetVsh(&shaderProgram, &shaderDvlb->DVLE[0]);
    C3D_BindProgram(&shaderProgram);
    uProjection = shaderInstanceGetUniformLocation(shaderProgram.vertexShader, "projection");
    uModelView = shaderInstanceGetUniformLocation(shaderProgram.vertexShader, "modelView");
    C3D_AttrInfo *attr = C3D_GetAttrInfo();
    AttrInfo_Init(attr);
    AttrInfo_AddLoader(attr, 0, GPU_FLOAT, 3);
    AttrInfo_AddLoader(attr, 1, GPU_FLOAT, 3);
    C3D_TexEnv *env = C3D_GetTexEnv(0);
    C3D_TexEnvInit(env);
    C3D_TexEnvSrc(env, C3D_Both, GPU_PRIMARY_COLOR, 0, 0);
    C3D_TexEnvFunc(env, C3D_Both, GPU_REPLACE);
    Mtx_PerspTilt(&projection, C3D_AngleFromDegrees(55.0f), 400.0f/240.0f, 0.1f, 100.0f, false);
    C3D_DepthTest(true, GPU_GEQUAL, GPU_WRITE_ALL);
    C3D_CullFace(GPU_CULL_NONE);
}
static void sceneExit(void) {
    shaderProgramFree(&shaderProgram);
    DVLB_Free(shaderDvlb);
}
int main(void) {
    gfxInitDefault();
    consoleInit(GFX_BOTTOM, &bottomConsole);
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    topTarget = C3D_RenderTargetCreate(240, 400, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);
    C3D_RenderTargetSetOutput(topTarget, GFX_TOP, GFX_LEFT, DISPLAY_TRANSFER_FLAGS);
    sceneInit();

    while (aptMainLoop()) {
        hidScanInput();
        u32 down = hidKeysDown();
        u32 held = hidKeysHeld();
        if (down & KEY_START) break;
        if ((held & KEY_UP) && stamina > 0.5f) { climbHeight += 0.018f; stamina -= 0.12f; }
        if (down & KEY_X) { climbHeight += 0.10f; stamina -= 3.0f; }
        if ((held & KEY_DOWN) || (held & KEY_Y)) stamina += 0.28f;
                if (down & KEY_A) leftGrip = !leftGrip;
        if (down & KEY_B) rightGrip = !rightGrip;
        if (!leftGrip && !rightGrip) { climbHeight -= 0.025f; stamina -= 0.1f; }
        if (stamina < 0.0f) stamina = 0.0f;
        if (stamina > 100.0f) stamina = 100.0f;
        if (climbHeight < 0.0f) climbHeight = 0.0f;

        touchPosition touch;
        if (held & KEY_TOUCH) {
            hidTouchRead(&touch);
            if (touch.px < 160) leftGrip = 1; else rightGrip = 1;
            if (touch.py < 115 && stamina > 0.4f) { climbHeight += 0.008f; stamina -= 0.08f; }
            if (touch.py >= 115) stamina += 0.12f;
        }

        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
        C3D_RenderTargetClear(topTarget, C3D_CLEAR_ALL, CLEAR_COLOR, 0x00000000);
        C3D_FrameDrawOn(topTarget);
        C3D_BindProgram(&shaderProgram);
        /* The camera rises as the scout climbs, giving a sense of upward progress. */
        Mtx_LookAt(&modelView, FVec3_New(0.0f, 1.6f + climbHeight*0.55f, 7.0f), FVec3_New(0.0f, 2.0f + climbHeight*0.55f, -5.0f), FVec3_New(0.0f, 1.0f, 0.0f), false);
        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uProjection, &projection);
        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uModelView, &modelView);
        drawMountain();
        drawHands();
        C3D_FrameEnd(0);
        updateBottom();
        gspWaitForVBlank();
    }
    sceneExit();
    C3D_Fini();
    gfxExit();
    return 0;
}
