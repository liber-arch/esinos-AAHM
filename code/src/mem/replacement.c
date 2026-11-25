#include <stdio.h>
#include "replacement.h"

/* Vérifie les arguments : pages >= 0 (la valeur -1 sert à marquer un cadre vide) */
static int args_ok(const int *refs, int n, int num_frames) {
    if (refs == NULL || n < 0 || num_frames <= 0 || num_frames > MAX_FRAMES)
        return 0;
    for (int i = 0; i < n; i++)
        if (refs[i] < 0)
            return 0;
    return 1;
}

/* Retourne l'index de page dans frames, ou -1 si absente.
   find_page(frames, num_frames, -1) sert à trouver un cadre libre. */
static int find_page(const int *frames, int num_frames, int page) {
    for (int j = 0; j < num_frames; j++)
        if (frames[j] == page)
            return j;
    return -1;
}

/* Affiche l'état des cadres après une référence */
static void print_state(int page, const int *frames, int num_frames, int hit) {
    printf("ref %d : ", page);
    for (int j = 0; j < num_frames; j++) {
        if (frames[j] == -1)
            printf("- ");
        else
            printf("%d ", frames[j]);
    }
    printf("%s\n", hit ? "(hit)" : "(defaut)");
}

/* ---------- FIFO : évince la page chargée en premier ---------- */
int fifo(const int *refs, int n, int num_frames, int verbose) {
    if (!args_ok(refs, n, num_frames))
        return -1;

    int frames[MAX_FRAMES];
    int next = 0;      /* cadre le plus ancien, donc prochain à remplacer */
    int faults = 0;

    for (int i = 0; i < num_frames; i++)
        frames[i] = -1;

    for (int i = 0; i < n; i++) {
        int hit = (find_page(frames, num_frames, refs[i]) != -1);

        if (!hit) {
            faults++;
            int free_idx = find_page(frames, num_frames, -1);
            if (free_idx != -1) {
                frames[free_idx] = refs[i];          /* cadre libre */
            } else {
                frames[next] = refs[i];              /* évince la plus ancienne */
                next = (next + 1) % num_frames;
            }
        }
        if (verbose)
            print_state(refs[i], frames, num_frames, hit);
    }
    return faults;
}

/* ---------- LRU : évince la page inutilisée depuis le plus longtemps ---------- */
int lru(const int *refs, int n, int num_frames, int verbose) {
    if (!args_ok(refs, n, num_frames))
        return -1;

    int frames[MAX_FRAMES];
    int last_used[MAX_FRAMES];   /* instant du dernier accès de chaque cadre */
    int faults = 0;

    for (int i = 0; i < num_frames; i++) {
        frames[i] = -1;
        last_used[i] = -1;
    }

    for (int i = 0; i < n; i++) {
        int idx = find_page(frames, num_frames, refs[i]);
        int hit = (idx != -1);

        if (hit) {
            last_used[idx] = i;                      /* mise à jour de l'accès */
        } else {
            faults++;
            idx = find_page(frames, num_frames, -1);
            if (idx == -1) {                         /* plus de cadre libre */
                idx = 0;
                for (int j = 1; j < num_frames; j++)
                    if (last_used[j] < last_used[idx])
                        idx = j;                     /* le plus ancien accès */
            }
            frames[idx] = refs[i];
            last_used[idx] = i;
        }
        if (verbose)
            print_state(refs[i], frames, num_frames, hit);
    }
    return faults;
}

/* ---------- Optimal : évince la page qui sera réutilisée le plus tard ---------- */

/* Prochain instant >= from où page est référencée, ou n si jamais réutilisée */
static int next_use(const int *refs, int n, int from, int page) {
    for (int k = from; k < n; k++)
        if (refs[k] == page)
            return k;
    return n;
}

int optimal(const int *refs, int n, int num_frames, int verbose) {
    if (!args_ok(refs, n, num_frames))
        return -1;

    int frames[MAX_FRAMES];
    int faults = 0;

    for (int i = 0; i < num_frames; i++)
        frames[i] = -1;

    for (int i = 0; i < n; i++) {
        int hit = (find_page(frames, num_frames, refs[i]) != -1);

        if (!hit) {
            faults++;
            int idx = find_page(frames, num_frames, -1);
            if (idx == -1) {                         /* plus de cadre libre */
                int farthest = -1;
                for (int j = 0; j < num_frames; j++) {
                    int d = next_use(refs, n, i + 1, frames[j]);
                    if (d > farthest) {
                        farthest = d;
                        idx = j;                     /* réutilisée le plus tard */
                    }
                }
            }
            frames[idx] = refs[i];
        }
        if (verbose)
            print_state(refs[i], frames, num_frames, hit);
    }
    return faults;
}