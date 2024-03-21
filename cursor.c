#pragma once 

#include "types.h"
#include "string.c"

typedef struct Cursor
{
    i32 pos;
    i32 desiredCol;
} Cursor;

i32 GetLineOffset(StringBuffer* buffer, Cursor* cursor, i32* lineStartOut)
{
    i32 index = IndexBefore(buffer, cursor->pos, '\n');
    i32 lineStart = index == -1 ? 0 : index + 1;
    if(lineStartOut)
        *lineStartOut = lineStart;
        
    return cursor->pos - lineStart;

}

void CursorMoveLeft(Cursor* cursor, StringBuffer* buffer)
{
    if(cursor->pos > 0)
    {
        cursor->pos--;
        cursor->desiredCol = GetLineOffset(buffer, cursor, 0);
    }
}

void CursorMoveRight(Cursor* cursor, StringBuffer* buffer)
{
    if(cursor->pos < buffer->size - 1)
    {
        cursor->pos++;
        cursor->desiredCol = GetLineOffset(buffer, cursor, 0);
    }
}

void CursorMoveDown(Cursor* cursor, StringBuffer* buffer)
{
    i32 nextLine = IndexAfter(buffer, cursor->pos - 1, '\n');
    if(nextLine != -1)
    {
        i32 target = nextLine + cursor->desiredCol + 1;
        i32 lineAfterNextLine = IndexAfter(buffer, nextLine, '\n');
        if(lineAfterNextLine == -1)
            lineAfterNextLine = buffer->size - 1;
            
        if(target > lineAfterNextLine)
            cursor->pos = lineAfterNextLine;
        else 
            cursor->pos = target;
    }
}

void CursorMoveUp(Cursor* cursor, StringBuffer* buffer)
{
    i32 index = IndexBefore(buffer, cursor->pos, '\n');
    i32 lineStart = index == -1 ? 0 : index + 1;

    if(index != -1)
    {
        i32 prevLine = IndexBefore(buffer, index, '\n');
        i32 target = prevLine + cursor->desiredCol + 1;
        if(target > lineStart)
            cursor->pos = lineStart - 1;
        else 
            cursor->pos = target;
    }
}
