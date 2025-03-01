/* -*- C++ -*-
 *
 *  PonscripterLabel_effect.cpp - Effect executer of Ponscripter
 *
 *  Copyright (c) 2001-2009 Ogapee (original ONScripter, of which this
 *  is a fork).
 *
 *  ogapee@aqua.dti2.ne.jp
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307 USA
 */

#include "PonscripterLabel.h"

#define EFFECT_STRIPE_WIDTH (16 * screen_ratio1 / screen_ratio2)
#define EFFECT_STRIPE_CURTAIN_WIDTH (24 * screen_ratio1 / screen_ratio2)
#define EFFECT_QUAKE_AMP (12 * screen_ratio1 / screen_ratio2)

static char *dll=NULL, *params=NULL; //for dll-based effects

int PonscripterLabel::setEffect(Effect& effect, bool generate_effect_dst, bool update_backup_surface)
{
    if (effect.effect == 0) return RET_CONTINUE;

    if (update_backup_surface)
        refreshSurface(backup_surface, &dirty_rect.bounding_box,
                       REFRESH_NORMAL_MODE);
    
    int effect_no = effect.effect;
    if (effect_cut_flag && skip_flag) effect_no = 1;

    SDL_BlitSurface(accumulation_surface, NULL, effect_src_surface, NULL);

    if (generate_effect_dst){
        int refresh_mode = refreshMode();
        if (update_backup_surface && refresh_mode == REFRESH_NORMAL_MODE){
            SDL_BlitSurface(backup_surface, &dirty_rect.bounding_box,
                            effect_dst_surface, &dirty_rect.bounding_box);
        }
        else {
            if (effect_no == 1)
                refreshSurface(effect_dst_surface, &dirty_rect.bounding_box,
                               refresh_mode);
            else
                refreshSurface(effect_dst_surface, NULL, refresh_mode);
        }
    }
    
    /* Load mask image */
    if (effect_no == 15 || effect_no == 18){
        if (!effect.anim.image_surface){
            parseTaggedString(&effect.anim, true);
            setupAnimationInfo(&effect.anim);
        }
    }
    if (effect_no == 11 || effect_no == 12 || effect_no == 13 ||
        effect_no == 14 || effect_no == 16 || effect_no == 17)
        dirty_rect.fill( screen_width, screen_height );

    dll = params = NULL;
    if (effect_no == 99) { // dll-based
        dll = bstr2cstr(&effect.anim.image_name, '0'); // TODO: Make dll a bstring natively
        if (dll != NULL) { //just in case no dll is given
            if (debug_level > 0)
                printf("dll effect: Got dll/params '%s'\n", dll);

            params = dll;
            while (*params != 0 && *params != '/') params++;
            if (*params == '/') params++;

            if (!strncmp(dll, "whirl.dll", 9)) {
                buildSinTable();
                buildCosTable();
                buildWhirlTable();
                dirty_rect.fill( screen_width, screen_height );
            }
            else if (!strncmp(dll, "trvswave.dll", 12)) {
                buildSinTable();
                dirty_rect.fill( screen_width, screen_height );
            }
            else if (!strncmp(dll, "breakup.dll", 11)) {
                initBreakup(params);
                dirty_rect.fill( screen_width, screen_height );
            }
            else {
                dirty_rect.fill( screen_width, screen_height );
            }
        }
    }

    effect_counter = 0;
    effect_start_time_old = SDL_GetTicks();
    event_mode = EFFECT_EVENT_MODE;
    advancePhase();

    return RET_WAIT | RET_REREAD;
}


int PonscripterLabel::doEffect(Effect& effect, bool clear_dirty_region)
{
    if (lastRenderEvent < RENDER_EVENT_EFFECT) { lastRenderEvent = RENDER_EVENT_EFFECT; }

    bool first_time = (effect_counter == 0);

    int prevduration = effect.duration;
    if (ctrl_pressed_status || skip_to_wait) {
        effect.duration = effect_counter = 1;
    }

    effect_start_time = SDL_GetTicks();

    effect_timer_resolution = effect_start_time - effect_start_time_old;
    effect_start_time_old = effect_start_time;

    int effect_no = effect.effect;
    if (effect_cut_flag && skip_flag) effect_no = 1;

    int i;
    int width, width2;
    int height, height2;
    SDL_Rect src_rect = { 0, 0, screen_width, screen_height };
    SDL_Rect dst_rect = { 0, 0, screen_width, screen_height };

    /* ---------------------------------------- */
    /* Execute effect */
    //printf("Effect number %d %d\n", effect_no, effect->duration );

    switch (effect_no) {
    case 0: // Instant display
    case 1: // Instant display
        //drawEffect( &src_rect, &src_rect, effect_dst_surface );
        break;

    case 2: // Left shutter
        width = EFFECT_STRIPE_WIDTH * effect_counter / effect.duration;
        for (i = 0; i < screen_width / EFFECT_STRIPE_WIDTH; i++) {
            src_rect.x = i * EFFECT_STRIPE_WIDTH;
            src_rect.y = 0;
            src_rect.w = width;
            src_rect.h = screen_height;
            drawEffect(&src_rect, &src_rect, effect_dst_surface);
        }

        break;

    case 3: // Right shutter
        width = EFFECT_STRIPE_WIDTH * effect_counter / effect.duration;
        for (i = 1; i <= screen_width / EFFECT_STRIPE_WIDTH; i++) {
            src_rect.x = i * EFFECT_STRIPE_WIDTH - width - 1;
            src_rect.y = 0;
            src_rect.w = width;
            src_rect.h = screen_height;
            drawEffect(&src_rect, &src_rect, effect_dst_surface);
        }

        break;

    case 4: // Top shutter
        height = EFFECT_STRIPE_WIDTH * effect_counter / effect.duration;
        for (i = 0; i < screen_height / EFFECT_STRIPE_WIDTH; i++) {
            src_rect.x = 0;
            src_rect.y = i * EFFECT_STRIPE_WIDTH;
            src_rect.w = screen_width;
            src_rect.h = height;
            drawEffect(&src_rect, &src_rect, effect_dst_surface);
        }

        break;

    case 5: // Bottom shutter
        height = EFFECT_STRIPE_WIDTH * effect_counter / effect.duration;
        for (i = 1; i <= screen_height / EFFECT_STRIPE_WIDTH; i++) {
            src_rect.x = 0;
            src_rect.y = i * EFFECT_STRIPE_WIDTH - height - 1;
            src_rect.w = screen_width;
            src_rect.h = height;
            drawEffect(&src_rect, &src_rect, effect_dst_surface);
        }

        break;

    case 6: // Left curtain
        width = EFFECT_STRIPE_CURTAIN_WIDTH * effect_counter * 2 / effect.duration;
        for (i = 0; i <= screen_width / EFFECT_STRIPE_CURTAIN_WIDTH; i++) {
            width2 = width - EFFECT_STRIPE_CURTAIN_WIDTH * EFFECT_STRIPE_CURTAIN_WIDTH * i / screen_width;
            if (width2 >= 0) {
                src_rect.x = i * EFFECT_STRIPE_CURTAIN_WIDTH;
                src_rect.y = 0;
                src_rect.w = width2;
                src_rect.h = screen_height;
                drawEffect(&src_rect, &src_rect, effect_dst_surface);
            }
        }

        break;

    case 7: // Right curtain
        width = EFFECT_STRIPE_CURTAIN_WIDTH * effect_counter * 2 / effect.duration;
        for (i = 0; i <= screen_width / EFFECT_STRIPE_CURTAIN_WIDTH; i++) {
            width2 = width - EFFECT_STRIPE_CURTAIN_WIDTH * EFFECT_STRIPE_CURTAIN_WIDTH * i / screen_width;
            if (width2 >= 0) {
                if (width2 > EFFECT_STRIPE_CURTAIN_WIDTH) width2 = EFFECT_STRIPE_CURTAIN_WIDTH;

                src_rect.x = screen_width - i * EFFECT_STRIPE_CURTAIN_WIDTH - width2;
                src_rect.y = 0;
                src_rect.w = width2;
                src_rect.h = screen_height;
                drawEffect(&src_rect, &src_rect, effect_dst_surface);
            }
        }

        break;

    case 8: // Top curtain
        height = EFFECT_STRIPE_CURTAIN_WIDTH * effect_counter * 2 / effect.duration;
        for (i = 0; i <= screen_height / EFFECT_STRIPE_CURTAIN_WIDTH; i++) {
            height2 = height - EFFECT_STRIPE_CURTAIN_WIDTH * EFFECT_STRIPE_CURTAIN_WIDTH * i / screen_height;
            if (height2 >= 0) {
                src_rect.x = 0;
                src_rect.y = i * EFFECT_STRIPE_CURTAIN_WIDTH;
                src_rect.w = screen_width;
                src_rect.h = height2;
                drawEffect(&src_rect, &src_rect, effect_dst_surface);
            }
        }

        break;

    case 9: // Bottom curtain
        height = EFFECT_STRIPE_CURTAIN_WIDTH * effect_counter * 2 / effect.duration;
        for (i = 0; i <= screen_height / EFFECT_STRIPE_CURTAIN_WIDTH; i++) {
            height2 = height - EFFECT_STRIPE_CURTAIN_WIDTH * EFFECT_STRIPE_CURTAIN_WIDTH * i / screen_height;
            if (height2 >= 0) {
                src_rect.x = 0;
                src_rect.y = screen_height - i * EFFECT_STRIPE_CURTAIN_WIDTH - height2;
                src_rect.w = screen_width;
                src_rect.h = height2;
                drawEffect(&src_rect, &src_rect, effect_dst_surface);
            }
        }

        break;

    case 99: // dll-based
        if (dll != NULL) {
            if (!strncmp(dll, "cascade.dll", 11)) {
                effectCascade(params, effect.duration);
                break;
            } else if (!strncmp(dll, "whirl.dll", 9)) {
                effectWhirl(params, effect.duration);
                break;
            } else if (!strncmp(dll, "trvswave.dll", 12)) {
                effectTrvswave(params, effect.duration);
                break;
            } else if (!strncmp(dll, "breakup.dll", 11)) {
                effectBreakup(params, effect.duration);
                break;
            } else {
                if (first_time) {
                    printf("Effect %d, DLL emulation not found: %s\n", effect_no, dll);
                }
            }
        } else { //just in case no dll is given
            if (first_time) {
                printf("effect No. %d, but no DLL name supplied.\n", effect_no);
            }
        }

        // fall through to default case

    default:
        if (first_time) {
            printf("effect No. %d is not implemented. Crossfade is substituted for that.\n", effect_no);
        }

    case 10: // Cross fade
        height = 256 * effect_counter / effect.duration;
        alphaMaskBlend(NULL, ALPHA_BLEND_CONST, height, &dirty_rect.bounding_box);
        break;

    case 11: // Left scroll
        width = screen_width * effect_counter / effect.duration;
        src_rect.x = 0;
        dst_rect.x = width;
        src_rect.y = dst_rect.y = 0;
        src_rect.w = dst_rect.w = screen_width - width;
        src_rect.h = dst_rect.h = screen_height;
        drawEffect(&dst_rect, &src_rect, effect_src_surface);

        src_rect.x = screen_width - width - 1;
        dst_rect.x = 0;
        src_rect.y = dst_rect.y = 0;
        src_rect.w = dst_rect.w = width;
        src_rect.h = dst_rect.h = screen_height;
        drawEffect(&dst_rect, &src_rect, effect_dst_surface);
        break;

    case 12: // Right scroll
        width = screen_width * effect_counter / effect.duration;
        src_rect.x = width;
        dst_rect.x = 0;
        src_rect.y = dst_rect.y = 0;
        src_rect.w = dst_rect.w = screen_width - width;
        src_rect.h = dst_rect.h = screen_height;
        drawEffect(&dst_rect, &src_rect, effect_src_surface);

        src_rect.x = 0;
        dst_rect.x = screen_width - width - 1;
        src_rect.y = dst_rect.y = 0;
        src_rect.w = dst_rect.w = width;
        src_rect.h = dst_rect.h = screen_height;
        drawEffect(&dst_rect, &src_rect, effect_dst_surface);
        break;

    case 13: // Top scroll
        width = screen_height * effect_counter / effect.duration;
        src_rect.x = dst_rect.x = 0;
        src_rect.y = 0;
        dst_rect.y = width;
        src_rect.w = dst_rect.w = screen_width;
        src_rect.h = dst_rect.h = screen_height - width;
        drawEffect(&dst_rect, &src_rect, effect_src_surface);

        src_rect.x = dst_rect.x = 0;
        src_rect.y = screen_height - width - 1;
        dst_rect.y = 0;
        src_rect.w = dst_rect.w = screen_width;
        src_rect.h = dst_rect.h = width;
        drawEffect(&dst_rect, &src_rect, effect_dst_surface);
        break;

    case 14: // Bottom scroll
        width = screen_height * effect_counter / effect.duration;
        src_rect.x = dst_rect.x = 0;
        src_rect.y = width;
        dst_rect.y = 0;
        src_rect.w = dst_rect.w = screen_width;
        src_rect.h = dst_rect.h = screen_height - width;
        drawEffect(&dst_rect, &src_rect, effect_src_surface);

        src_rect.x = dst_rect.x = 0;
        src_rect.y = 0;
        dst_rect.y = screen_height - width - 1;
        src_rect.w = dst_rect.w = screen_width;
        src_rect.h = dst_rect.h = width;
        drawEffect(&dst_rect, &src_rect, effect_dst_surface);
        break;

    case 15: // Fade with mask
        alphaMaskBlend(effect.anim.image_surface, ALPHA_BLEND_FADE_MASK, 256 * effect_counter / effect.duration, &dirty_rect.bounding_box);
        break;

    case 16: // Mosaic out
        generateMosaic(effect_src_surface, 5 - 6 * effect_counter / effect.duration);
        break;

    case 17: // Mosaic in
        generateMosaic(effect_dst_surface, 6 * effect_counter / effect.duration);
        break;

    case 18: // Cross fade with mask
        alphaMaskBlend(effect.anim.image_surface, ALPHA_BLEND_CROSSFADE_MASK, 256 * effect_counter * 2 / effect.duration, &dirty_rect.bounding_box);
        break;

    case (CUSTOM_EFFECT_NO + 0): // quakey
        if (effect_timer_resolution > effect.duration / 4 / effect.no)
            effect_timer_resolution = effect.duration / 4 / effect.no;

        dst_rect.x = 0;
        dst_rect.y = (Sint16) (sin(M_PI * 2.0 * effect.no * effect_counter / effect.duration) *
                               EFFECT_QUAKE_AMP * effect.no * (effect.duration - effect_counter) / effect.duration);
        SDL_FillRect(accumulation_surface, NULL, SDL_MapRGBA(accumulation_surface->format, 0, 0, 0, 0xff));
        drawEffect(&dst_rect, &src_rect, effect_dst_surface);
        break;

    case (CUSTOM_EFFECT_NO + 1): // quakex
        if (effect_timer_resolution > effect.duration / 4 / effect.no)
            effect_timer_resolution = effect.duration / 4 / effect.no;

        dst_rect.x = (Sint16) (sin(M_PI * 2.0 * effect.no * effect_counter / effect.duration) *
                               EFFECT_QUAKE_AMP * effect.no * (effect.duration - effect_counter) / effect.duration);
        dst_rect.y = 0;
        drawEffect(&dst_rect, &src_rect, effect_dst_surface);
        break;

    case (CUSTOM_EFFECT_NO + 2): // quake
        dst_rect.x = effect.no * get_rnd(-1, 1) * 2;
        dst_rect.y = effect.no * get_rnd(-1, 1) * 2;
        SDL_FillRect(accumulation_surface, NULL, SDL_MapRGBA(accumulation_surface->format, 0, 0, 0, 0xff));
        drawEffect(&dst_rect, &src_rect, effect_dst_surface);
        break;
    }

    //printf("effect conut %d / dur %d\n", effect_counter, effect.duration);

    effect_counter += effect_timer_resolution;
    if (effect_counter < effect.duration && effect_no != 1) {
        if (effect_no)
            flush(REFRESH_NONE_MODE, NULL, false);
        effect.duration = prevduration;
        return RET_WAIT | RET_REREAD;
    }
    else {
        SDL_BlitSurface(effect_dst_surface, &dirty_rect.bounding_box,
			accumulation_surface, &dirty_rect.bounding_box);

        if (effect_no)
	    flush(REFRESH_NONE_MODE, NULL, clear_dirty_region);
        if (effect_no == 1)
	    effect_counter = 0;

        effect.duration = prevduration;
        event_mode = IDLE_EVENT_MODE;

        return RET_CONTINUE;
    }
}


void PonscripterLabel::drawEffect(SDL_Rect* dst_rect, SDL_Rect* src_rect, SDL_Surface* surface)
{
    SDL_Rect clipped_rect;
    if (AnimationInfo::doClipping(dst_rect, &dirty_rect.bounding_box, &clipped_rect)) return;

    if (src_rect != dst_rect) {
        src_rect->x += clipped_rect.x;
        src_rect->y += clipped_rect.y;
        src_rect->w  = clipped_rect.w;
        src_rect->h  = clipped_rect.h;
    }

    SDL_BlitSurface(surface, src_rect, accumulation_surface, dst_rect);
}


void PonscripterLabel::generateMosaic(SDL_Surface* src_surface, int level)
{
    int i, j, ii, jj;
    int width = 160;
    for (i = 0; i < level; i++) width >>= 1;

#ifdef BPP16
    int total_width = accumulation_surface->pitch / 2;
#else
    int total_width = accumulation_surface->pitch / 4;
#endif
    SDL_LockSurface(src_surface);
    SDL_LockSurface(accumulation_surface);
    ONSBuf* src_buffer = (ONSBuf*) src_surface->pixels;

    for (i = screen_height - 1; i >= 0; i -= width) {
        for (j = 0; j < screen_width; j += width) {
            ONSBuf  p = src_buffer[i * total_width + j];
            ONSBuf* dst_buffer = (ONSBuf*) accumulation_surface->pixels + i * total_width + j;

            int height2 = width;
            if (i + 1 - width < 0) height2 = i + 1;

            int width2 = width;
            if (j + width > screen_width) width2 = screen_width - j;

            for (ii = 0; ii < height2; ii++) {
                for (jj = 0; jj < width2; jj++) {
                    *dst_buffer++ = p;
                }

                dst_buffer -= total_width + width2;
            }
        }
    }

    SDL_UnlockSurface(accumulation_surface);
    SDL_UnlockSurface(src_surface);
}
