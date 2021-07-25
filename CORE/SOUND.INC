{*--------------------------------------------------------------------------*}
{*                            S O U N D . I N C                             *}
{*--------------------------------------------------------------------------*}
{*                                                                          *}
{* Purposes   : Noise maker for the core game engine                        *}
{* Prequisites: CRT unit (sound/nosound)                                    *}
{*                                                                          *}
{*--------------------------------------------------------------------------*}
{* Origin     : Jani Nyk„nen, 2019                                          *}
{* Conversion : Trinh D.D. Nguyen, 2021                                     *}
{*--------------------------------------------------------------------------*}

{$ifndef __CORE_SOUND__}
{$define __CORE_SOUND__}

{*--------------------------- I N T E R F A C E ----------------------------*}

const	SND_BEEP1	= 0;
	SND_BEEP2	= 1;
	SND_BEEP3	= 2;
	SND_BEEP4	= 3;
	SND_BEEP5	= 4;
	SND_BREAK	= 5;
	SND_MOVE	= 6;
	SND_ACTIVATE	= 7;
	SND_DISAPPEAR	= 8;
	SND_ITEM	= 9;
	SND_VICTORY	= 10;
	SND_EXPLOSION	= 11;

{ SOUND API }
procedure	SND_init;			forward;
procedure	SND_play(snd: uint8);           forward;
procedure       SND_update(steps: int16);       forward;
procedure	SND_toggle;                     forward;
function	SND_is_enabled: boolean;        forward;

{*---------------------- I M P L E M E N T A T I O N -----------------------*}

const   S_BEEP1	: array[0..2]  of int16 = (125, 20, -1);
	S_BEEP2	: array[0..2]  of int16 = (200, 8, -1);
        S_BEEP3	: array[0..2]  of int16 = (150, 15, -1);
        S_BEEP4	: array[0..2]  of int16 = (100, 15, -1);
        S_BEEP5	: array[0..2]  of int16 = (200, 5, -1);
        S_BREAK	: array[0..6]  of int16 = (100, 5, 0, 4, 150, 10, -1);
        S_MOVE	: array[0..2]  of int16 = (200, 5, -1);
        S_ACTIVE: array[0..6]  of int16 = (200, 5, 0, 4, 150, 5, -1);
        S_DISAPR: array[0..6]  of int16 = (200, 4, 150, 5, 100, 10, -1);
        S_ITEM	: array[0..6]  of int16 = (200, 4, 250, 5, 300, 10, -1);
        S_VICT	: array[0..10] of int16 = (200, 20, 150, 5, 250, 5, 175, 5, 300, 30, -1);
        S_EXPLSN: array[0..14] of int16 = (100, 4, 125, 4, 150, 4, 125, 4, 100, 4, 75, 5, 50, 20, -1);

var	SND_enabled	: boolean;
	SND_playing	: boolean;
        SND_buffer	: pint16;
        SND_timer	: uint16;
        SND_length	: uint16;
	SND_pointer	: uint16;

procedure	SND_init;
begin
	SND_enabled	:= TRUE;
        SND_playing	:= FALSE;
        SND_buffer	:= nil;
        SND_timer	:= 0;
        SND_length	:= 0;
end;

procedure	SND_play(snd: uint8);
begin
	if not SND_enabled then exit;
        case snd of
        SND_BEEP1:      SND_buffer := @S_BEEP1;
        SND_BEEP2:	SND_buffer := @S_BEEP2;
        SND_BEEP3:	SND_buffer := @S_BEEP3;
        SND_BEEP4:	SND_buffer := @S_BEEP4;
        SND_BEEP5:	SND_buffer := @S_BEEP5;
        SND_BREAK:	SND_buffer := @S_BREAK;
        SND_MOVE:	SND_buffer := @S_MOVE;
        SND_ACTIVATE:	SND_buffer := @S_ACTIVE;
        SND_DISAPPEAR:	SND_buffer := @S_DISAPR;
        SND_ITEM:	SND_buffer := @S_ITEM;
        SND_VICTORY:	SND_buffer := @S_VICT;
        SND_EXPLOSION:	SND_buffer := @S_EXPLSN;
        end;

        SND_playing := TRUE;
        SND_timer   := 0;
        SND_pointer := 0;

        nosound;
        sound(uint16(SND_buffer^[0]));
        SND_length := uint16(SND_buffer^[1]);
end;

procedure       SND_update(steps: int16);
begin
        if (not SND_enabled) or (not SND_playing) then exit;

        inc(SND_timer, uint16(steps));
        if (SND_timer >= SND_length) then
        begin
        	dec(SND_timer, SND_length);
                inc(SND_pointer);
                nosound;
                if SND_buffer^[SND_pointer * 2] = -1 then
                begin
                	SND_playing := FALSE;
                	exit;
                end;
                if SND_buffer^[SND_pointer] > 0 then
                	sound(uint16(SND_buffer^[SND_pointer * 2]));
                SND_length := uint16(SND_buffer^[SND_pointer * 2 + 1]);
        end;
end;

procedure	SND_toggle;
begin
	SND_enabled := not SND_enabled;
        if not SND_enabled and SND_playing then
        begin
        	SND_playing := FALSE;
                nosound;
        end;
end;

function	SND_is_enabled: boolean;
begin
        SND_is_enabled := SND_enabled;
end;

{$endif}