{*--------------------------------------------------------------------------*}
{*                             I N T R O. I N C                             *}
{*--------------------------------------------------------------------------*}
{*                                                                          *}
{* Purposes   : Introduction scene                                          *}
{* Prequisites: GLOBALS.INC, Core game engine                               *}
{*                                                                          *}
{*--------------------------------------------------------------------------*}
{* Origin     : Jani Nyk„nen, 2019                                          *}
{* Conversion : Trinh D.D. Nguyen, 2021                                     *}
{*--------------------------------------------------------------------------*}

{$ifndef __PLUTONIUM_CAVERNS_INTRO__}
{$define __PLUTONIUM_CAVERNS_INTRO__}

const	INTRO_WAIT_TIME	= 120;
	INTRO_WAIT_MUL	= 2;

var     bmp_intro	: BITMAP;
	bmp_drawn	: boolean;
        intro_timer	: int16;

function	intro_init: boolean;                    forward;
procedure	intro_update(steps: int16);             forward;
procedure	intro_draw;                             forward;
procedure	intro_change(param: pointer);           forward;
procedure	intro_dispose;                          forward;

procedure	intro_goto_title;
begin
	app_change_scene(SCENE_TITLE, pointer(int16(1)));
end;

function	intro_init: boolean;
begin
	intro_init := FALSE;
	bmp_intro  := VGA_load_bitmap(F_BMP_INTRO);
        if bmp_intro = nil then exit;

        TRN_activate(FADE_OUT, 2, nil);

        bmp_drawn   := FALSE;
        intro_timer := INTRO_WAIT_TIME;
        intro_init  := TRUE;
end;

procedure	intro_update(steps: int16);
begin
	if TRN_is_active then exit;
        dec(intro_timer, steps);
        if intro_timer <= 0 then
		TRN_activate(FADE_IN, 2, intro_goto_title);
end;

procedure	intro_draw;
begin
	if not bmp_drawn then
        begin
                VGA_clear(0);
                VGA_draw_bitmap_fast(bmp_intro,
				     160 - bmp_intro^.w shr 1,
				     100 - bmp_intro^.h shr 1);
                bmp_drawn := TRUE;
        end;
end;

procedure	intro_dispose;
begin
	VGA_destroy_bitmap(bmp_intro);
end;

procedure	intro_change(param: pointer);
begin
        bmp_intro   := VGA_load_bitmap(F_BMP_END);
        if bmp_intro = nil then APP_terminate;
        intro_timer := INTRO_WAIT_TIME * INTRO_WAIT_MUL;
        bmp_drawn   := FALSE;
end;

function	intro_setup_scene: SCENE;
var	s: SCENE;
begin
	new(s);
        with s^ do
        begin
		init	:= intro_init;
        	update  := intro_update;
        	draw	:= intro_draw;
        	dispose := intro_dispose;
        	change  := intro_change;
        	name    := SCENE_INTRO;
	end;
        intro_setup_scene := s;
end;

{$endif}
