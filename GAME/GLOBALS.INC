{*--------------------------------------------------------------------------*}
{*                          G L O B A L S . I N C                           *}
{*--------------------------------------------------------------------------*}
{*                                                                          *}
{* Purposes   : Game globals definitions                                    *}
{* Prequisites: None                                                        *}
{*                                                                          *}
{*--------------------------------------------------------------------------*}
{* Origin     : Jani Nyk„nen, 2019                                          *}
{* Conversion : Trinh D.D. Nguyen, 2021                                     *}
{*--------------------------------------------------------------------------*}

{$ifndef __PLUTONIUM_CAVERNS_GLOBALS__}
{$define __PLUTONIUM_CAVERNS_GLOBALS__}

const   ASSETS_ROOT	= 'ASSETS\';
        ASSETS_BITMAPS	= ASSETS_ROOT + 'BITMAPS\';
        ASSETS_MAPS	= ASSETS_ROOT + 'MAPS\';

        F_BMP_ANIM	= ASSETS_BITMAPS + 'ANIM.BIN'   ;
        F_BMP_END	= ASSETS_BITMAPS + 'END.BIN'    ;
        F_BMP_EXPLOSION	= ASSETS_BITMAPS + 'EXP.BIN'    ;
        F_BMP_FONT	= ASSETS_BITMAPS + 'FONT.BIN'   ;
        F_BMP_FRAME	= ASSETS_BITMAPS + 'FRAME.BIN'  ;
        F_BMP_INTRO	= ASSETS_BITMAPS + 'INTRO.BIN'  ;
        F_BMP_ITEMS	= ASSETS_BITMAPS + 'ITEMS.BIN'  ;
	F_BMP_LOGO	= ASSETS_BITMAPS + 'LOGO.BIN'   ;
	F_BMP_PLAYER	= ASSETS_BITMAPS + 'PLAYER.BIN' ;
	F_BMP_SHIP	= ASSETS_BITMAPS + 'SHIP.BIN'   ;
	F_BMP_SMENU	= ASSETS_BITMAPS + 'SMENU.BIN'  ;
	F_BMP_STORY1	= ASSETS_BITMAPS + 'STORY1.BIN' ;
	F_BMP_STORY2	= ASSETS_BITMAPS + 'STORY2.BIN' ;
	F_BMP_TILESET	= ASSETS_BITMAPS + 'TILESET.BIN';

	F_SAVE_GAME	= 'SAVE.DAT';

        SCENE_INTRO	= 'intro';
        SCENE_TITLE	= 'title';
        SCENE_SMENU	= 'stagemenu';
        SCENE_GAME	= 'game';
        SCENE_STORY	= 'story';

{$endif}
