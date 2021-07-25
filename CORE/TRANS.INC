{*--------------------------------------------------------------------------*}
{*                             T R A N S. I N C                             *}
{*--------------------------------------------------------------------------*}
{*                                                                          *}
{* Purposes   : Scene transitions for the core game engine                  *}
{* Prequisites: TYPES.INC, GRAPHICS.INC                                     *}
{*                                                                          *}
{*--------------------------------------------------------------------------*}
{* Origin     : Jani Nyk„nen, 2019                                          *}
{* Conversion : Trinh D.D. Nguyen, 2021                                     *}
{*--------------------------------------------------------------------------*}

{$ifndef __CORE_TRANSITION__}
{$define __CORE_TRANSITION__}

{*--------------------------- I N T E R F A C E ----------------------------*}

const	FADE_IN		= 0;
	FADE_OUT	= 1;
        TRN_INIT_TIME	= 70;

type	TRN_MODE	= uint8;
	TRN_CALLBACK	= procedure;

{ TRANSITION API }
procedure	TRN_init;                                                       forward;
procedure	TRN_activate(mode: TRN_MODE; speed: int16; cb: TRN_CALLBACK);   forward;
procedure	TRN_update(steps: int16);                                       forward;
procedure	TRN_draw;                                                       forward;
function	TRN_is_active: boolean;                                         forward;

{*---------------------- I M P L E M E N T A T I O N -----------------------*}

var	TRN_current	: TRN_MODE;
	TRN_timer	: int16;
        TRN_speed	: int16;
        TRN_active	: boolean;
        TRN_cb		: TRN_CALLBACK;

procedure	TRN_init;
begin
        TRN_active := FALSE;
        TRN_cb	   := nil;
end;

procedure	TRN_activate(mode: TRN_MODE; speed: int16; cb: TRN_CALLBACK);
begin
	TRN_current := mode;
        TRN_speed   := speed;
        TRN_active  := TRUE;
        TRN_cb      := cb;
        TRN_timer   := TRN_INIT_TIME;
end;

procedure	TRN_update(steps: int16);
begin
        if not TRN_active then exit;

        dec(TRN_timer, steps * TRN_speed);
        if TRN_timer <= 0 then
        begin
        	if TRN_current = FADE_OUT then
                begin
                	TRN_timer := 0;
                        VGA_set_palette_darkness(0);
                        TRN_active := FALSE;
                end
                else
                begin
                	inc(TRN_timer, TRN_INIT_TIME);
                        if @TRN_cb <> nil then TRN_cb;
			TRN_current := FADE_OUT;
                end;
        end;
end;

procedure	TRN_draw;
var	level	: int16;
begin
	if not TRN_active then exit;

        level := TRN_timer div 10;
        if TRN_current = FADE_IN then level := 7-level;

        VGA_set_palette_darkness(level);
end;

function	TRN_is_active: boolean;
begin
	TRN_is_active := TRN_active;
end;

{$endif}