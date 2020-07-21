static b32
IsWhitespace(char C){
    b32 Result = false;

    static char WhitespaceChars[] = {
        '\t',
        '\n',
        '\r',
        ' '
    };

    for(u32 W=0;
            W < StaticArraySize(WhitespaceChars);
            W++){
        if(WhitespaceChars[W] == C){
            Result = true;
            break;
        }
    }

    return Result;
}

inline b32
IsNumeric(char C){
    b32 Result = false;

    if(C >= '0' && C <= '9'){
        Result = true;
    }

    return Result;
}

static size_t
StringLength(char *Str){
    // NOTE(furkan): This function assumes Str to be null-terminated

    size_t Result = 0;

    while(Str[Result] != '\0'){
        Result++;
    }

    return Result;
}

static s32
StringCompare(char *A, size_t LengthA, char *B, size_t LengthB){
    s32 Result = 0;

    if(LengthA == LengthB){
        for(size_t i=0;
                   i < LengthA;
                   i++){
            if(A[i] < B[i]){
                Result = -1;
                break;
            } else if (A[i] > B[i]){
                Result = 1;
                break;
            }
        }
    } else {
        Result = (LengthA < LengthB) ? -1 : 1;
    }

    return Result;
}

static u32
StringToU32(char *Str, char **CursorEnd=0){
    u32 Result = 0;

    while(IsWhitespace(Str[0])){
        Str++;
    }

    while(true){
        if(Str[0] == '\0' || IsWhitespace(Str[0])){
            break;
        } else {
            Assert(Str[0] >= '0' && Str[0] <= '9');

            Result *= 10;
            Result += Str[0] - '0';
            Str++;
        }
    }

    if(CursorEnd){
        *CursorEnd = Str;
    }

    return Result;
}

static s32
StringToS32(char *Str, char **CursorEnd=0){
    s32 Result = 0;

    while(IsWhitespace(Str[0])){
        Str++;
    }

    b32 IsNegative = false;
    if(Str[0] == '-'){
        IsNegative = true;
        Str++;
    }

    while(true){
        if(Str[0] == '\0' || IsWhitespace(Str[0])){
            break;
        } else {
            Assert(Str[0] >= '0' && Str[0] <= '9');

            Result *= 10;
            Result += Str[0] - '0';
            Str++;
        }
    }

    if(IsNegative){
        Result = -Result;
    }

    if(CursorEnd){
        *CursorEnd = Str;
    }

    return Result;
}

static f32
StringToF32(char *Str, char **CursorEnd=0){
    f32 Result = 0.0f;

    while(IsWhitespace(Str[0])){
        Str++;
    }

    b32 IsNegative = false;
    if(Str[0] == '-'){
        IsNegative = true;
        Str++;
    }

    s32 Wholes = 0;
    while(true){
        if(Str[0] =='\0' || IsWhitespace(Str[0]) || Str[0] == '.' || 
           Str[0] == 'e' || Str[0] == 'E'){
            break;
        } else {
            Assert(Str[0] >= '0' && Str[0] <= '9');

            Wholes *= 10;
            Wholes += Str[0] - '0';
            Str++;
        }
    }

    f32 FracFactor = 1.0f;
    f32 Frac = 0.0f;
    if(Str[0] == '.'){
        Str++;
    
        while(true){
            if(Str[0] == '\0' || IsWhitespace(Str[0]) ||
               Str[0] == 'e' || Str[0] == 'E'){
                break;
            } else {
                Assert(Str[0] >= '0' && Str[0] <= '9');

                FracFactor *= 10;
                Frac *= 10;
                Frac += Str[0] - '0';
                Str++;
            }
        }
    }

    Result = ((f32)Wholes) + (Frac/FracFactor);

    if(Str[0] == 'e' || Str[0] == 'E'){
        Str++;
    
        b32 NegativeExponent = false;
        if(Str[0] == '-'){
            NegativeExponent = true;
            Str++;
        } else if(Str[0] == '+'){
             Str++;
        }
        
        f32 Exponent = 0.0f;
        while(true){
            if(Str[0] == '\0' || IsWhitespace(Str[0])){
                break;
            } else {
                Assert(Str[0] >= '0' && Str[0] <= '9');

                Exponent *= 10;
                Exponent += Str[0] - '0';
                Str++;
            }
        }

        if(NegativeExponent){
            Exponent *= -1.0f;
        }

        f32 Factor = Power(10.0f, Exponent);
        Result *= Factor;
    }

    if(IsNegative){
        Result *= -1.0f;
    }

    if(CursorEnd){
        *CursorEnd = Str;
    }

    return Result;
}

static vec2
StringToVec2(char *Str, char **CursorEnd=0){
    vec2 Result = Vec2(0.0f, 0.0f);

    Result.x = StringToF32(Str, &Str); 
    Result.y = StringToF32(Str, &Str); 

    if(CursorEnd){
        *CursorEnd = Str;
    }

    return Result;
}

static vec2u
StringToVec2u(char *Str, char **CursorEnd=0){
    vec2u Result = Vec2u(0, 0);

    Result.x = StringToU32(Str, &Str); 
    Result.y = StringToU32(Str, &Str); 

    if(CursorEnd){
        *CursorEnd = Str;
    }

    return Result;
}

static vec3
StringToVec3(char *Str, char **CursorEnd=0){
    vec3 Result = Vec3(0.0f, 0.0f, 0.0f);

    Result.x = StringToF32(Str, &Str); 
    Result.y = StringToF32(Str, &Str); 
    Result.z = StringToF32(Str, &Str); 

    if(CursorEnd){
        *CursorEnd = Str;
    }

    return Result;
}

static vec3u
StringToVec3u(char *Str, char **CursorEnd=0){
    vec3u Result = Vec3u(0, 0, 0);

    Result.x = StringToU32(Str, &Str); 
    Result.y = StringToU32(Str, &Str); 
    Result.z = StringToU32(Str, &Str); 

    if(CursorEnd){
        *CursorEnd = Str;
    }

    return Result;
}

static vec4
StringToVec4(char *Str, char **CursorEnd=0){
    vec4 Result = Vec4(0.0f, 0.0f, 0.0f, 0.0f);

    Result.x = StringToF32(Str, &Str); 
    Result.y = StringToF32(Str, &Str); 
    Result.z = StringToF32(Str, &Str); 
    Result.w = StringToF32(Str, &Str); 

    if(CursorEnd){
        *CursorEnd = Str;
    }

    return Result;
}

static char *
StringNextWord(char *Cursor, size_t *WordLen, char **CursorEnd=0){

    char *Result = 0;
    size_t Len = 0;

    // NOTE(furkan): Result will be null if 
    // Cursor points to the end of string

    while(IsWhitespace(Cursor[0]) && Cursor[0] != '\0'){
        Cursor++;
    }

    if(Cursor[0] != '\0'){
        Result = Cursor;

        while(!IsWhitespace(Cursor[0]) && Cursor[0] != '\0'){
            Cursor++;
            Len++;
        }
    }

    if(CursorEnd){
        *CursorEnd = Cursor;
    }

    *WordLen = Len;

    return Result;
}

static char *
StringNextLine(char *Cursor, size_t *LineLen, char **CursorEnd=0){

    char *Result = Cursor;
    size_t Len = 0;

    while(Cursor[0] != '\n' && Cursor[0] != '\0'){
        Cursor++;
        if(Cursor[0] != '\r'){
            Len++;
        }
    }

    if(Cursor[0] != '\0'){
        Cursor++;
    }

    if(Len == 0){
        Result = 0;
    }

    *LineLen = Len;

    if(CursorEnd){
        *CursorEnd = Cursor;
    }

    return Result;
}
