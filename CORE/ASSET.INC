{*--------------------------------------------------------------------------*}
{*                             A S S E T . I N C                            *}
{*--------------------------------------------------------------------------*}
{*                                                                          *}
{* Purposes   : Asset management for the core game engine                   *}
{* Prequisites: TYPES.INC, GRAPHICS.INC, TILEMAP.INC                        *}
{*                                                                          *}
{*--------------------------------------------------------------------------*}
{* Origin     : Jani Nyk„nen, 2019                                          *}
{* Conversion : Trinh D.D. Nguyen, 2021                                     *}
{*--------------------------------------------------------------------------*}

{$ifndef __CORE_ASSET__}
{$define __CORE_ASSET__}

{*--------------------------- I N T E R F A C E ----------------------------*}

const   ASSET_MAX	= 64;
	ASSET_NAMELEN	= 16;

const	AST_BITMAP	= 0;
	AST_TILEMAP	= 1;

type	ASSET = record
                link	: pointer;
                name	: string[ASSET_NAMELEN];
                atype	: int16;
                empty	: boolean;
	end;

{ ASSET MANAGEMENT API }
function	AST_find_empty: int16;                                             forward;
function	AST_put(link: pointer; name: string; atype: int16): boolean;       forward;
function	AST_add_bitmap(path, name: string): boolean;                       forward;
function	AST_add_tilemap(path, name: string): boolean;                      forward;
function        AST_get(name: string): pointer;                                    forward;
procedure	AST_remove(name: string);                                          forward;
procedure	AST_init;                                                          forward;
procedure	AST_destroy;                                                       forward;

{*---------------------- I M P L E M E N T A T I O N -----------------------*}

var	AST_buffer	: array[0..ASSET_MAX-1] of ASSET;

function	AST_find_empty: int16;
var	i	: int16;
begin
	i := 0;
        while (i < ASSET_MAX) do
        begin
        	if AST_buffer[i].empty then
                begin
                	AST_find_empty := i;
                	exit;
                end;
        	inc(i);
        end;
        AST_find_empty := -1;
end;

function	AST_put(link: pointer; name: string; atype: int16): boolean;
var	ass	: ASSET;
	index	: int16;
begin
	AST_put   := FALSE;
	index     := AST_find_empty;
        if index = -1 then exit;

        ass.link  := link;
        ass.name  := name;
        ass.atype := atype;
        ass.empty := FALSE;

        AST_buffer[index] := ass;
	AST_put   := TRUE;
end;

procedure	AST_init;
var	i	: int16;
begin
	for i := 0 to ASSET_MAX-1 do AST_buffer[i].empty := TRUE;
end;

function	AST_add_bitmap(path, name: string): boolean;
var	bmp	: BITMAP;
begin
	AST_add_bitmap := FALSE;

	bmp := VGA_load_bitmap(path);
        if bmp = nil then exit;

        if not AST_put(bmp, name, AST_BITMAP) then
        begin
        	VGA_destroy_bitmap(bmp);
                exit;
        end;
        AST_add_bitmap := TRUE;
end;

function	AST_add_tilemap(path, name: string): boolean;
var	t	: TILEMAP;
begin
	AST_add_tilemap := FALSE;

	t := TLM_load(path);
        if t = nil then exit;

        if not AST_put(t, name, AST_TILEMAP) then
        begin
        	TLM_destroy(t);
                exit;
        end;
        AST_add_tilemap := TRUE;
end;

function        AST_get(name: string): pointer;
var	i	: int16;
begin
        AST_get := nil;
	i := 0;
        while i < ASSET_MAX do
        begin
                if not AST_buffer[i].empty and
                   (name = AST_buffer[i].name) then
                begin
                        AST_get := AST_buffer[i].link;
                	exit;
                end;
        	inc(i);
        end;
end;

procedure	AST_remove(name: string);
var	i	: int16;
begin
        for i := 0 to ASSET_MAX-1 do
        begin
        	if (AST_buffer[i].empty) or
                   (AST_buffer[i].name <> name) then continue;

                case AST_buffer[i].atype of
                AST_BITMAP: 	VGA_destroy_bitmap(AST_buffer[i].link);
                AST_TILEMAP:	TLM_destroy(AST_buffer[i].link);
                end;

                AST_buffer[i].link  := nil;
                AST_buffer[i].empty := TRUE;
        end;
end;

procedure	AST_destroy;
var	i	: int16;
begin
        for i := 0 to ASSET_MAX-1 do
        begin
        	if AST_buffer[i].empty then continue;

                case AST_buffer[i].atype of
                AST_BITMAP: 	VGA_destroy_bitmap(AST_buffer[i].link);
                AST_TILEMAP:	TLM_destroy(AST_buffer[i].link);
                end;
        end;
end;

{$endif}