{*--------------------------------------------------------------------------*}
{*                               A P P. I N C                               *}
{*--------------------------------------------------------------------------*}
{*                                                                          *}
{* Purposes   : Application framework the core game engine                  *}
{* Prequisites: TYPES.INC, GRAPHICS.INC, ASSET.INC, INPUT.INC,              *}
{*              SOUND.INC, TRANS.INC                                        *}
{*                                                                          *}
{*--------------------------------------------------------------------------*}
{* Origin     : Jani Nyk„nen, 2019                                          *}
{* Conversion : Trinh D.D. Nguyen, 2021                                     *}
{*--------------------------------------------------------------------------*}

{$ifndef __CORE_APPLICATION__}
{$define __CORE_APPLICATION__}

{*--------------------------- I N T E R F A C E ----------------------------*}

{ APPLICATION API }
procedure	APP_dispose;						forward;
procedure	APP_update(steps: int16);                               forward;
procedure	APP_draw;                                               forward;
function	APP_init: boolean;                                      forward;
procedure	APP_done;                                               forward;
procedure	APP_run;                                                forward;
procedure	APP_add_scene(s: SCENE; active: boolean);               forward;
procedure	APP_change_scene(name: string; param: pointer);         forward;
procedure	APP_terminate;                                          forward;

{*---------------------- I M P L E M E N T A T I O N -----------------------*}

const	APP_MAX_SCENES	= 16;

var	APP_scenes	: array[0..APP_MAX_SCENES-1] of SCENE;
	APP_scene_count	: uint8;
        APP_active	: SCENE;
        APP_frame_skip	: int16;
        APP_step_count	: int16;
        APP_running	: boolean;

procedure	APP_dispose;
var	i	: int16;
begin
	for i := 0 to APP_scene_count-1 do if APP_scenes[i] <> nil then
	begin
		if @APP_scenes[i]^.dispose <> nil then
			APP_scenes[i]^.dispose;
                dispose(APP_scenes[i])
        end;
        AST_destroy;
        VGA_destroy_graphics;
        KBD_destroy;
end;

procedure	APP_update(steps: int16);
begin
        if @APP_active^.update <> nil then APP_active^.update(steps);
        TRN_update(steps);
        SND_update(steps);
end;

procedure	APP_draw;
begin
	if @APP_active^.draw <> nil then APP_active^.draw;
        TRN_draw;
end;

function	APP_init: boolean;
var	i	: uint16;
begin
	APP_init := FALSE;

	MLC_init;
        ERR_init;
	if not VGA_init_graphics then
	begin
        	ERR_throw_no_param('Graphics initialized error.');
		exit;
        end;

        KBD_init;
        AST_init;
        TRN_init;
        SND_init;

        for i := 0 to APP_MAX_SCENES-1 do APP_scenes[i] := nil;
        APP_scene_count := 0;
        APP_frame_skip  := 1;
        APP_init        := TRUE;
end;

procedure	APP_run;
var	update	: boolean;
	i	: int16;
begin
        APP_running := TRUE;
	update      := FALSE;

        for i := 0 to APP_scene_count-1 do
	begin
                if not APP_scenes[i]^.init then
                begin
                	APP_running := FALSE;
                        break;
                end;
        end;

	APP_step_count := 0;
        while (APP_running) do
        begin
        	update := FALSE;
                inc(APP_step_count);

                if (APP_frame_skip = 0) or
		   (APP_step_count > APP_frame_skip) then
                begin
                	APP_step_count := 0;
                        update := TRUE;
                end;
                if update and (APP_active <> nil) then
                begin
                	app_update(APP_frame_skip + 1);
                        app_draw;
                end;
                VGA_wait_retrace;
                VGA_draw_frame;
        end;

end;

procedure	APP_done;
begin
        APP_dispose;

        if ERR_get <> '' then
		writeln('ERROR: ', ERR_get);

	MLC_done;
end;

procedure	APP_add_scene(s: SCENE; active: boolean);
begin
	if APP_scene_count = APP_MAX_SCENES then
        	ERR_throw_no_param('Maximum scenes reached.');

        APP_scenes[APP_scene_count] := s;
        inc(APP_scene_count);
        if active then APP_active := APP_scenes[APP_scene_count-1];
end;

procedure	APP_change_scene(name: string; param: pointer);
var	i	: int16;
begin
	for i := 0 to APP_scene_count-1 do
        begin
                if APP_scenes[i]^.name = name then
                begin
                	APP_active := APP_scenes[i];
                        if @APP_active^.change <> nil then
				APP_active^.change(param);
                        exit;
                end;
        end;
end;

procedure	APP_terminate;
begin
	APP_running := FALSE;
end;

{$endif}