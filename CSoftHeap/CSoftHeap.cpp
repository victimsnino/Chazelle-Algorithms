// MIT License
// 
// Copyright (c) 2020 Aleksey Loginov
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "CSoftHeap.h"

#include <cstdlib>

struct ilcell
{
    int            key{};
    struct ilcell* next{};
};


struct node
{
    int            ckey{}, rank{};
    struct node* next{}, * child{};
    struct ilcell* il{}, * il_tail{};
};

struct head
{
    struct node* queue{};
    struct head* next{}, * prev{}, * suffix_min{};
    int          rank{};
};

#define INFTY 100000

CSoftHeap::CSoftHeap(int r)
    : m_r(r)
{
    m_header = new head();
    m_tail   = new head();

    m_tail->rank   = INFTY;
    m_header->next = m_tail;
    m_tail->prev   = m_header;
}

void CSoftHeap::insert(int new_key)
{
    node*   q;
    ilcell* l;
    l          = (ilcell*)malloc(sizeof(ilcell));
    l->key     = new_key;
    l->next    = NULL;
    q          = new node();
    q->rank    = 0;
    q->ckey    = new_key;
    q->il      = l;
    q->il_tail = l;

    meld(q);
}

int CSoftHeap::deletemin()
{
    node *tmp;
    int   min,   childcount;
    head* h = m_header->next->suffix_min;
    while (h->queue->il == NULL)
    {
        tmp        = h->queue;
        childcount = 0;
        while (tmp->next != NULL)
        {
            tmp = tmp->next;
            childcount++;
        }

        // remeld childs
        if (childcount < h->rank / 2)
        {
            h->prev->next = h->next;
            h->next->prev = h->prev;
            fix_minlist(h->prev);
            tmp = h->queue;
            while (tmp->next != NULL)
            {
                meld(tmp->child);
                tmp = tmp->next;
            }
        }
        else
        {
            h->queue = sift(h->queue);
            if (h->queue->ckey == INFTY)
            {
                h->prev->next = h->next;
                h->next->prev = h->prev;
                h             = h->prev;
            }
            fix_minlist(h);
        }
        h = m_header->next->suffix_min;
    } /* end of outer while loop */

    // Actually remove
    min          = h->queue->il->key;
    h->queue->il = h->queue->il->next;
    if (h->queue->il == NULL)
        h->queue->il_tail = NULL;
    return min;
}

void CSoftHeap::meld(node* q)
{
    head* tohead = m_header->next;
    while (q->rank > tohead->rank)
        tohead = tohead->next;

    head* prevhead = tohead->prev;

    node *top, *bottom;
    while (q->rank == tohead->rank)
    {
        if (tohead->queue->ckey > q->ckey)
        {
            top    = q;
            bottom = tohead->queue;
        }
        else
        {
            top    = tohead->queue;
            bottom = q;
        }
        q          = new node();
        q->ckey    = top->ckey;
        q->rank    = top->rank + 1;
        q->child   = bottom;
        q->next    = top;
        q->il      = top->il;
        q->il_tail = top->il_tail;
        tohead     = tohead->next;
    }

    head* h;
    if (prevhead == tohead->prev)
        h = new head();
    else // actually we've moved out queue from this one...
        h = prevhead->next;
    h->queue       = q;
    h->rank        = q->rank;
    h->prev        = prevhead;
    h->next        = tohead;
    prevhead->next = h;
    tohead->prev   = h;

    fix_minlist(h);
}

void CSoftHeap::fix_minlist(head* h)
{
    head* tmpmin;
    if (h->next == m_tail)
        tmpmin = h;
    else
        tmpmin = h->next->suffix_min;
    while (h != m_header)
    {
        if (h->queue->ckey < tmpmin->queue->ckey)
            tmpmin = h;
        h->suffix_min = tmpmin;
        h = h->prev;
    }
}

node* CSoftHeap::sift(node* v)
{
    node* tmp;
    v->il      = NULL;
    v->il_tail = NULL;
    if (v->next == NULL && v->child == NULL)
    {
        v->ckey = INFTY;
        return v;
    }
    v->next = sift(v->next);

    // It is possible that v->next has INFTY ckey. Swap it in this case 
    if (v->next->ckey > v->child->ckey)
    {
        tmp      = v->child;
        v->child = v->next;
        v->next  = tmp;
    }

    // Move item list from v->next to v
    v->il      = v->next->il;
    v->il_tail = v->next->il_tail;
    v->ckey    = v->next->ckey;

    // Sometimes we can do it twice due branching
    if (v->rank > m_r &&
        (v->rank % 2 == 1 || v->child->rank < v->rank - 1))
    {
        v->next = sift(v->next);

        // Do it again
        if (v->next->ckey > v->child->ckey)
        {
            tmp      = v->child;
            v->child = v->next;
            v->next  = tmp;
        }

        // Concatenate lists if not empty
        if (v->next->ckey != INFTY &&
            v->next->il != NULL)
        {
            v->next->il_tail->next = v->il;
            v->il                  = v->next->il;
            if (v->il_tail == NULL)
                v->il_tail = v->next->il_tail;
            v->ckey = v->next->ckey;
        }
    } /*  end of second sift */

    // Clean Up
    if (v->child->ckey == INFTY)
    {
        if (v->next->ckey == INFTY)
        {
            v->child = NULL;
            v->next  = NULL;
        }
        else
        {
            v->child = v->next->child;
            v->next  = v->next->next;
        }
    }
    return v;
}
