{*--------------------------------------------------------------------------*}
{*                           T I L E M A P . I N C                          *}
{*--------------------------------------------------------------------------*}
{*                                                                          *}
{* Purposes   : Sprite management for the core game engine                  *}
{* Prequisites: TYPES.INC, GRAPHICS.INC                                     *}
{*                                                                          *}
{*--------------------------------------------------------------------------*}
{* Origin     : Jani Nyk„nen, 2019                                          *}
{* Conversion : Trinh D.D. Nguyen, 2021                                     *}
{*--------------------------------------------------------------------------*}

{$ifndef __CORE_TILEMAP__}
{$define __CORE_TILEMAP__}

{*--------------------------- I N T E R F A C E ----------------------------*}

const	TILE_MAP_LAYERS	= 16;

type    TILEMAP = ^TILEMAP_TYPE;
	TILEMAP_TYPE = record
		w, h	: uint16;
                count	: uint8;
                layers	: array[0..TILE_MAP_LAYERS-1] of RASTER;
	end;

{ TILEMAP API }
function	TLM_load(filename: string): TILEMAP;			     forward;
procedure	TLM_destroy(t: TILEMAP);                                     forward;
function	TLM_get_tile(t: TILEMAP; layer: uint8; x, y: int16): uint8;  forward;

{*---------------------- I M P L E M E N T A T I O N -----------------------*}

function	TLM_load(filename: string): TILEMAP;
var	t	: TILEMAP;
	i	: uint8;
        f	: FILE;
begin
	TLM_load := nil;
        if not file_open(f, FALSE, filename) then exit;
        new(t);
        file_read(t^.w, sizeof(uint16), f);
        file_read(t^.h, sizeof(uint16), f);
        file_read(t^.count, sizeof(uint8), f);
        for i := 0 to t^.count-1 do
        begin
                t^.layers[i] := MEM_malloc(t^.w * t^.h);
                if t^.layers[i] = nil then
                begin
                	dispose(t);
                	file_close(f);
                        exit;
                end;
                file_read(t^.layers[i]^, t^.w * t^.h, f);
        end;
        file_close(f);
        TLM_load := t;
end;

procedure	TLM_destroy(t: TILEMAP);
var	i	: uint8;
begin
	if t = nil then exit;
        for i := 0 to t^.count-1 do MEM_free(t^.layers[i]);
        dispose(t);
end;

function	TLM_get_tile(t: TILEMAP; layer: uint8; x, y: int16): uint8;
begin
        TLM_get_tile := 0;
        if t = nil then exit;
        if (layer >= t^.count) or
	   (x < 0) or (y < 0) or
           (x >= int16(t^.w)) or (y >= int16(t^.h)) then exit;
        TLM_get_tile := t^.layers[layer]^[y * t^.w + x];
end;

{$endif}