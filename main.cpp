#include "pch.h"
//============================== 
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include <stdexcept>
#include <string>
#include <vector>
#include <set>
#include <map>
using namespace std;
//============================== 
#define _TO_STRING(e) #e
#define TO_STRING(e) _TO_STRING(e)
#define FILE_LINE __FILE__ "(" TO_STRING(__LINE__) ")"
#define ASSERT(b) do { if(!(b)) throw logic_error(FILE_LINE " : assert failed ! " #b); } while(0)
#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
static string unescape(const string &s) {
    string r;
    for (int i = 0; i < (int)s.size(); ++i) {
        if (s[i] == '\\') {
            switch (s[++i]) {
                case 't': r += '\t'; break;
                case 'n': r += '\n'; break;
                case 'r': r += '\r'; break;
                default: r += s[i]; break;
            }
        } else r += s[i];
    }
    return r;
}
static string format(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    static vector<char> buf(256);
    while ((int)buf.size() == vsnprintf(&buf[0], buf.size(), fmt, args)) {
        buf.resize(buf.size() * 3 / 2);
    }
    va_end(args);
    return &buf[0];
}
static string readFile(const string &fileName) {
    string r;
    FILE *f = fopen(fileName.c_str(), "rb");
    if (f == NULL) return r;
    fseek(f, 0, SEEK_END);
    int size = ftell(f);
    fseek(f, 0, SEEK_SET);
    r.resize(size + 1, 0);
    int bytes = (int)fread(&r[0], size, 1, f);
    ASSERT(bytes == 1);
    fclose(f);
    return r;
}
//============================== lexical analysis
enum TokenID {
    TID_LP, TID_RP, TID_LBRACE, TID_RBRACE, TID_LBRACKET, TID_RBRACKET, TID_COMMA, TID_SEMICELON,
    TID_IF, TID_ELSE, TID_FOR, TID_WHILE, TID_CONTINUE, TID_BREAK, TID_RETURN,
    TID_OP_NOT, TID_OP_INC, TID_OP_DEC,
    TID_OP_ASSIGN,
    TID_OP_AND, TID_OP_OR,
    TID_OP_ADD, TID_OP_SUB, TID_OP_MUL, TID_OP_DIV, TID_OP_MOD,   
    TID_OP_LESS, TID_OP_LESSEQ, TID_OP_GREATER, TID_OP_GREATEREQ, TID_OP_EQUAL, TID_OP_NEQUAL, 
    TID_TYPE_INT, TID_TYPE_STRING, TID_TYPE_VOID,
    TID_TRUE, TID_FALSE,
    // special 
    TID_INT, TID_ID, TID_STRING,
    TID_EOF,
};
struct Token {
    TokenID tid;
    string lexeme;
    Token(): tid(TID_EOF) {}
    explicit Token(TokenID _tt): tid(_tt) {}
    Token(TokenID _tt, const char *begin, const char *end): tid(_tt), lexeme(begin, end){}
};
static map<string, Token> setupBuildinTokens() {
    map<string, Token> tokens;
    const char *lexemes[] = {
        "(", ")", "{", "}", "[", "]", ",", ";",
        "if", "else", "for", "while", "continue", "break", "return",
        "!", "++", "--",
        "=",
        "&&", "||",
        "+", "-", "*", "/", "%", 
        "<", "<=", ">", ">=", "==", "!=",
        "int", "string", "void",
        "true", "false",
    };
    for (int i = 0; i < ARRAY_SIZE(lexemes); ++i) tokens[lexemes[i]] = Token((TokenID)i);
    return tokens; 
}
static Token* getBuildinToken(const string &lexeme) {
    static map<string, Token> s_tokens(setupBuildinTokens());
    map<string, Token>::iterator iter = s_tokens.find(lexeme);
    return iter == s_tokens.end() ? NULL : &iter->second;
}
class Scanner { 
public:
    explicit Scanner(const char *src): m_src(src) { }
    Token LA(int n) { fetchN(n); return m_LAList[n - 1]; }
    Token next(int n) {
        fetchN(n);
        Token token = m_LAList[0];
        m_LAList.erase(m_LAList.begin(), m_LAList.begin() + n);
        return token;
    }
private:
    void fetchN(int n) {
        while ((int)m_LAList.size() < n) {
            for (; isspace(*m_src); ++m_src);

            if (m_src[0] == 0) {
                m_LAList.push_back(Token());
            } else if (m_src[0] == '/' && m_src[1] == '/') {
                while (*++m_src != '\n');
            } else if (m_src[0] == '/' && m_src[1] == '*') {
                ++m_src;
                do { ++m_src; } while(m_src[0] != '*' || m_src[1] != '/');
                m_src += 2;
            } else if (isdigit(m_src[0])) {
                const char *begin = m_src;
                while (isdigit(*++m_src));
                m_LAList.push_back(Token(TID_INT, begin, m_src));
            } else if (m_src[0] == '"') {
                const char *begin = m_src;
                while (*++m_src != '"') {
                    if (*m_src == '\\') ++m_src;
                }
                m_LAList.push_back(Token(TID_STRING, begin, ++m_src));
            } else if (isalpha(m_src[0]) || m_src[0] == '_') {
                const char *begin = m_src;
                do { ++m_src; } while(isalpha(m_src[0]) || m_src[0] == '_' || isdigit(m_src[0]));
                string lexeme(begin, m_src);
                if (Token *token = getBuildinToken(lexeme)) m_LAList.push_back(*token);
                else m_LAList.push_back(Token(TID_ID, begin, m_src));
            } else {
                string lexeme(m_src, m_src + 2);
                if (Token *token = getBuildinToken(lexeme)) {
                    m_LAList.push_back(*token);
                    m_src += 2;
                } else {
                    lexeme.assign(m_src, m_src + 1);
                    if (Token *token = getBuildinToken(lexeme)) {
                        m_LAList.push_back(*token);
                        ++m_src;
                    } else ASSERT(0 && "invalid token");
                }
            }
        }
    }
private:
    vector<Token> m_LAList;
    const char *m_src;
};
//============================== platform details for jit
#ifdef _MSC_VER
#pragma warning(disable : 4312)
#pragma warning(disable : 4311)
#include <Windows.h>
#include <Psapi.h>
#undef TokenID
#pragma comment(lib, "Psapi.lib")
#pragma warning(default : 4311)
#pragma warning(default : 4312)
static char* os_mallocExecutable(int size) {
    char *p = (char*)::VirtualAlloc(NULL, size, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    ASSERT(p != NULL);
    return p;
}
static void os_freeExecutable(char *p) {
    ASSERT(p != NULL);
	::VirtualFree(p, 0, MEM_RELEASE);
}
static char* os_findSymbol(const char *funcName) {
    HANDLE process = ::OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, GetCurrentProcessId());
    ASSERT(process != NULL);

    vector<HMODULE> modules;
    {
        DWORD bytes;
        ::EnumProcessModules(process, NULL, 0, &bytes);
        modules.resize(bytes / sizeof(modules[0]));
        ::EnumProcessModules(process, &modules[0], bytes, &bytes);
    }
    ASSERT(modules.size() > 0);

    char *func = NULL;
    for (int i = 0; i < (int)modules.size(); ++i) {
        if (func = (char*)::GetProcAddress(modules[i], funcName)) {
            break;
        }
    }

    ::CloseHandle(process);
    return func;
}
#endif
//==============================
#if defined(__linux__) || defined(__APPLE__)
#include <dlfcn.h>
#include <unistd.h>
#include <sys/mman.h>
static char* os_mallocExecutable(int size) {
    char *p = NULL;
    int erro = ::posix_memalign((void**)&p, ::getpagesize(), size);
    ASSERT(erro == 0 && p != NULL);
    erro = ::mprotect(p, size, PROT_READ | PROT_WRITE | PROT_EXEC);
    ASSERT(erro == 0);
    return p;
}
static void os_freeExecutable(char *p) {
    ASSERT(p != NULL);
    ::free(p);
}
static char* os_findSymbol(const char *funcName) {
    void *m = ::dlopen(NULL, RTLD_NOW);
    char *r = (char*)::dlsym(m, funcName);
    ::dlclose(m);
    return r;
}
#endif
//============================== code generator
#define MAX_TEXT_SECTION_SIZE (4096 * 8)
#define MAX_LOCAL_COUNT 64
class x86FunctionBuilder;
class x86JITEngine {
public:
    x86JITEngine(): m_textSectionSize(0) {
        m_textSection = os_mallocExecutable(MAX_TEXT_SECTION_SIZE);
    }
    ~x86JITEngine() { os_freeExecutable(m_textSection); }
    char* getFunction(const string &name) { return *_getFunctionEntry(name); }

    void beginBuild();
    char** _getFunctionEntry(const string &name) { return &m_funcEntries[name]; }
    const char* _getLiteralStringLoc(const string &literalStr) { return m_literalStrs.insert(literalStr).first->c_str();}
    x86FunctionBuilder* beginBuildFunction();
    void endBuildFunction(x86FunctionBuilder *builder);
    void endBuild();
private:
    char *m_textSection;
    int m_textSectionSize;
    map<string, char*> m_funcEntries;
    set<string> m_literalStrs;
};
class x86Label { 
public:
    x86Label(): m_address(NULL){}
    ~x86Label() { ASSERT(m_address != NULL); }
    void mark(char *address) {
        ASSERT(m_address == NULL);
        m_address = address;
        bindRefs();
    }
    void addRef(char *ref) {
        m_refs.push_back(ref);
        bindRefs();
    }
private:
    void bindRefs() {
        if (m_address == NULL) return;
        for (int i = 0; i < (int)m_refs.size(); ++i) {
            *(int*)m_refs[i] = int(m_address - (m_refs[i] + 4));
        }
        m_refs.clear();
    }
private:
    char *m_address;
    vector<char*> m_refs;
};
class x86FunctionBuilder {
public:
    x86FunctionBuilder(x86JITEngine *parent, char *codeBuf): m_parent(parent), m_codeBuf(codeBuf), m_codeSize(0){}
    string& getFuncName() { return m_funcName;}
    int getCodeSize() const{ return m_codeSize;}

    void beginBuild(){
        emit(1, 0x52); // push edx
        emit(1, 0x55); // push ebp
        emit(2, 0x8b, 0xec); // mov ebp, esp
        emit(2, 0x81, 0xec); emitValue(MAX_LOCAL_COUNT * 4); // sub esp, MAX_LOCAL_COUNT * 4
    }
    void endBuild(){
        markLabel(&m_retLabel);
        emit(2, 0x8b, 0xe5);  // mov esp,ebp 
        emit(1, 0x5d); // pop ebp  
        emit(1, 0x5a); // pop edx  
        emit(1, 0xc3); // ret
    }

    void loadImm(int imm){
        emit(1, 0x68); emitValue(imm); // push imm
    }
    void loadLiteralStr(const string &literalStr){
        const char *loc = m_parent->_getLiteralStringLoc(literalStr);
        emit(1, 0x68); emitValue(loc); // push loc
    }
    void loadLocal(int idx){
        emit(2, 0xff, 0xb5); emitValue(localIdx2EbpOff(idx)); // push dword ptr [ebp + idxOff]
    }
    void storeLocal(int idx) {
        emit(3, 0x8b, 0x04, 0x24); // mov eax, dword ptr [esp]
        emit(2, 0x89, 0x85); emitValue(localIdx2EbpOff(idx)); // mov dword ptr [ebp + idxOff], eax
        emit(2, 0x83, 0xc4); emitValue((char)4); // add esp, 4
    }
    void incLocal(int idx) {
        emit(2, 0xff, 0x85); emitValue(localIdx2EbpOff(idx)); // inc dword ptr [ebp + idxOff]
    }
    void decLocal(int idx) {
        emit(2, 0xff, 0x8d); emitValue(localIdx2EbpOff(idx)); // dec dword ptr [ebp + idxOff]
    }
    void pop(int n){
        emit(2, 0x81, 0xc4); emitValue(n * 4); // add esp, n * 4
    }
    void dup(){
        emit(3, 0xff, 0x34, 0x24); // push dword ptr [esp]
    }

    void doArithmeticOp(TokenID opType) {
        emit(4, 0x8b, 0x44, 0x24, 0x04); // mov eax, dword ptr [esp+4]
        switch (opType) {
            case TID_OP_ADD: 
                emit(3, 0x03, 0x04, 0x24); // add eax, dword ptr [esp]
                break;
            case TID_OP_SUB: 
                emit(3, 0x2b, 0x04, 0x24); // sub eax, dword ptr [esp]
                break;
            case TID_OP_MUL: 
                emit(4, 0x0f, 0xaf, 0x04, 0x24); // imul eax, dword ptr [esp]
                break;
            case TID_OP_DIV: 
            case TID_OP_MOD: 
                emit(2, 0x33, 0xd2); // xor edx, edx
                emit(3, 0xf7, 0x3c, 0x24); // idiv dword ptr [esp]
                if (opType == TID_OP_MOD) {
                    emit(2, 0x8b, 0xc2); // mov eax, edx
                }
                break;  
            default: ASSERT(0); break;
        }
        emit(4, 0x89, 0x44, 0x24, 0x04); // mov dword ptr [esp+4], eax
        emit(3, 0x83, 0xc4, 0x04); // add esp, 4
    }
    void cmp(TokenID cmpType) {
        x86Label label_1, label_0, label_end;
        emit(4, 0x8b, 0x44, 0x24, 0x04); // mov eax, dword ptr [esp+4] 
        emit(3, 0x8b, 0x14, 0x24); // mov edx, dword ptr[esp]
        emit(2, 0x83, 0xc4); emitValue((char)8);// add esp, 8
        emit(2, 0x3b, 0xc2); // cmp eax, edx
        condJmp(cmpType, &label_1);
        jmp(&label_0);
        markLabel(&label_1);
        emit(2, 0x6a, 0x01); // push 1
        jmp(&label_end);
        markLabel(&label_0);
        emit(2, 0x6a, 0x00); // push 0
        markLabel(&label_end);
    }

    void markLabel(x86Label *label){ label->mark(m_codeBuf + m_codeSize); }
    void jmp(x86Label *label) { 
        emit(1, 0xe9);
        char *ref = m_codeBuf + m_codeSize;
        emitValue(NULL);
        label->addRef(ref); 
    }
    void trueJmp(x86Label *label) {
        emit(3, 0x8b, 0x04, 0x24); // mov eax, dword ptr [esp]
        emit(3, 0x83, 0xc4, 0x04); // add esp, 4
        emit(2, 0x85, 0xc0); // test eax, eax
        condJmp(TID_OP_NEQUAL, label); 
    }
    void falseJmp(x86Label *label) {
        emit(3, 0x8b, 0x04, 0x24); // mov eax, dword ptr [esp]
        emit(3, 0x83, 0xc4, 0x04); // add esp, 4
        emit(2, 0x85, 0xc0); // test eax, eax
        condJmp(TID_OP_EQUAL, label); 
    }
    void ret() { jmp(&m_retLabel); }
    void retExpr() {
        emit(3, 0x8b, 0x04, 0x24); // mov eax, dword ptr [esp]
        emit(3, 0x83, 0xc4, 0x04); // add esp, 4
        jmp(&m_retLabel);
    }

    int beginCall(){
        return 0;
    }
    void endCall(const string &funcName, int callID, int paramCount){
        char ** entry = m_parent->_getFunctionEntry(funcName);
        for (int i = 0; i < paramCount - 1; ++i) {
            emit(3, 0xff, 0xb4, 0x24); emitValue(((i + 1) * 2 - 1) * 4); // push dword ptr [esp+4*i]
        }
        emit(2, 0xff, 0x15); emitValue(entry); // call [entry]
        pop(paramCount + (paramCount > 0 ? paramCount - 1 : 0));
        emit(1, 0x50); // push eax
    }
private:
    void emit(int n, ...) {
        va_list args;
        va_start(args, n);
        for (int i = 0; i < n; ++i) m_codeBuf[m_codeSize++] = (char)va_arg(args, int);
        va_end(args);
    }
    template<typename T>
    void emitValue(T val) {
        memcpy(m_codeBuf + m_codeSize, &val, sizeof(val));
        m_codeSize += sizeof(val);
    }
private:
    void condJmp(TokenID tid, x86Label *label) {
        switch (tid) {
            case TID_OP_LESS: emit(2, 0x0f, 0x8c); break;
            case TID_OP_LESSEQ: emit(2, 0x0f, 0x8e); break;
            case TID_OP_GREATER: emit(2, 0x0f, 0x8f); break;
            case TID_OP_GREATEREQ: emit(2, 0x0f, 0x8d); break;
            case TID_OP_EQUAL: emit(2, 0x0f, 0x84); break;
            case TID_OP_NEQUAL: emit(2, 0x0f, 0x85); break;
        }
        char *ref = m_codeBuf + m_codeSize;
        emitValue(NULL);
        label->addRef(ref);
    }
private:
    int localIdx2EbpOff(int idx) { return idx < 0 ? 8 - idx * 4 : -(1 + idx) * 4; }
private:
    x86JITEngine *m_parent;
    char *m_codeBuf;
    int m_codeSize;
    string m_funcName;
    x86Label m_retLabel;
};
void x86JITEngine::beginBuild() { }
x86FunctionBuilder* x86JITEngine::beginBuildFunction() {
    x86FunctionBuilder *r = new x86FunctionBuilder(this, m_textSection + m_textSectionSize);
    r->beginBuild();
    return r;
}
void x86JITEngine::endBuildFunction(x86FunctionBuilder *builder) {
    builder->endBuild();
    *_getFunctionEntry(builder->getFuncName()) = m_textSection + m_textSectionSize;
    m_textSectionSize += builder->getCodeSize();
    ASSERT(m_textSectionSize <= MAX_TEXT_SECTION_SIZE);
    delete builder;
}
void x86JITEngine::endBuild() {
    for (map<string, char*>::iterator iter = m_funcEntries.begin(); iter != m_funcEntries.end(); ++iter) {
        if (iter->second == NULL) {
            char *f = os_findSymbol(iter->first.c_str());
            ASSERT(f != NULL);
            iter->second = f;
        }
    }
}
//============================== syntax analysis
class FunctionParser {
public:
    FunctionParser(x86FunctionBuilder *builder, Scanner *scanner): m_builder(builder), m_scanner(scanner) {}
    void parse() { _function_define(); }
private:
    void _function_define() {
        m_scanner->next(1); // type
        m_builder->getFuncName() = m_scanner->next(1).lexeme;
        ASSERT(m_scanner->next(1).tid == TID_LP);
        while (m_scanner->LA(1).tid != TID_RP) {
            string type = m_scanner->next(1).lexeme;
            declareArg(m_scanner->next(1).lexeme, type);
            if (m_scanner->LA(1).tid == TID_COMMA) m_scanner->next(1);
        }
        ASSERT(m_scanner->next(1).tid == TID_RP);
        _block();
    }
    void _block() {
        pushScope();
        ASSERT(m_scanner->next(1).tid == TID_LBRACE);
        while (m_scanner->LA(1).tid != TID_RBRACE) _statement();
        ASSERT(m_scanner->next(1).tid == TID_RBRACE);
        popScope();
    }
    void _statement() {
        switch (m_scanner->LA(1).tid) {
            case TID_SEMICELON: m_scanner->next(1); break;
            case TID_CONTINUE: m_scanner->next(2); m_builder->jmp(getLastContinueLabel()); break;
            case TID_BREAK: m_scanner->next(2); m_builder->jmp(getLastBreakLabel()); break;
            case TID_RETURN: 
                m_scanner->next(1);
                if (m_scanner->LA(1).tid != TID_SEMICELON) {
                    _expr(0);
                    m_builder->retExpr();
                } else m_builder->ret();
                ASSERT(m_scanner->next(1).tid == TID_SEMICELON);
                break;
            case TID_LBRACE: _block(); break;
            case TID_IF: _if(); break;
            case TID_WHILE: _while(); break;
            case TID_FOR: _for(); break;
            case TID_TYPE_STRING: case TID_TYPE_INT: _local_define_list(); break;
            case TID_TYPE_VOID: ASSERT(0); break;
            default: _expr(0); m_builder->pop(1); ASSERT(m_scanner->next(1).tid == TID_SEMICELON); break;
        }
    }
    void _if() {
        x86Label label_true, label_false, label_end;

        m_scanner->next(2);
        _expr(0);
        ASSERT(m_scanner->next(1).tid == TID_RP);
        m_builder->trueJmp(&label_true);
        m_builder->jmp(&label_false);

        m_builder->markLabel(&label_true);
        _statement();
        m_builder->jmp(&label_end);

        m_builder->markLabel(&label_false);
        if (m_scanner->LA(1).tid == TID_ELSE) {
            m_scanner->next(1);
            _statement();
        }

        m_builder->markLabel(&label_end);
    }
    void _while() {
        x86Label *label_break = pushBreakLabel(), *label_continue = pushContinueLabel();

        m_builder->markLabel(label_continue);
        m_scanner->next(2);
        _expr(0);
        ASSERT(m_scanner->next(1).tid == TID_RP);
        m_builder->falseJmp(label_break);

        _statement();
        m_builder->jmp(label_continue);
        m_builder->markLabel(label_break);

        popBreakLabel(); popContinueLabel();
    }
    void _for() {
        pushScope();
        x86Label *label_continue = pushContinueLabel(), *label_break = pushBreakLabel();
        x86Label label_loop, label_body;

        m_scanner->next(2);
        switch (m_scanner->LA(1).tid) {
            case TID_SEMICELON: break;
            case TID_TYPE_INT: case TID_TYPE_STRING: _local_define_list(); break;
            default: _expr(0); m_builder->pop(1); ASSERT(m_scanner->next(1).tid == TID_SEMICELON); break;
        }

        m_builder->markLabel(&label_loop);
        if (m_scanner->LA(1).tid != TID_SEMICELON) _expr(0);
        else m_builder->loadImm(1);
        ASSERT(m_scanner->next(1).tid == TID_SEMICELON);
        m_builder->falseJmp(label_break);
        m_builder->jmp(&label_body);

        m_builder->markLabel(label_continue);
        if (m_scanner->LA(1).tid != TID_RP) {
            _expr(0); 
            m_builder->pop(1);
        }
        ASSERT(m_scanner->next(1).tid == TID_RP);
        m_builder->jmp(&label_loop);

        m_builder->markLabel(&label_body);
        _statement();
        m_builder->jmp(label_continue);

        m_builder->markLabel(label_break);
        popContinueLabel(); popBreakLabel();
        popScope();
    }
    void _local_define_list() {
        string type = m_scanner->next(1).lexeme;
        _id_or_assignment(type);
        while (m_scanner->LA(1).tid == TID_COMMA) {
            m_scanner->next(1);
            _id_or_assignment(type);
        }
        ASSERT(m_scanner->next(1).tid == TID_SEMICELON);
    }
    void _id_or_assignment(const string &type) {
        Token idToken = m_scanner->next(1);
        declareLocal(idToken.lexeme, type);
        if (m_scanner->LA(1).tid == TID_OP_ASSIGN) {
            m_scanner->next(1);
            _expr(0);
            m_builder->storeLocal(getLocal(idToken.lexeme));
        }
    }
    void _expr(int rbp) {
        if (m_scanner->LA(1).tid == TID_ID && m_scanner->LA(2).tid == TID_OP_ASSIGN) {
            Token idToken = m_scanner->next(2);
            _expr(0);
            m_builder->dup();
            m_builder->storeLocal(getLocal(idToken.lexeme));
        } else {
            _expr_nud();
            while (rbp < getOperatorLBP(m_scanner->LA(1).tid)) _expr_led();
        }
    }
    void _expr_nud() {
        Token token = m_scanner->next(1);
        switch (token.tid) {
            case TID_TRUE: m_builder->loadImm(1); break;
            case TID_FALSE: m_builder->loadImm(0); break;
            case TID_INT: m_builder->loadImm(atoi(token.lexeme.c_str())); break;
            case TID_STRING: m_builder->loadLiteralStr(unescape(token.lexeme.substr(1, token.lexeme.size() - 2))); break;
            case TID_ID: 
                if (m_scanner->LA(1).tid == TID_LP) _expr_call(token);
                else m_builder->loadLocal(getLocal(token.lexeme)); 
                break;
            case TID_LP: _expr(0); ASSERT(m_scanner->next(1).tid == TID_RP); break;
            case TID_OP_NOT: 
                 _expr(getOperatorRBP(token.tid)); 
                 m_builder->loadImm(0);
                 m_builder->cmp(TID_OP_EQUAL);
                 break;
            case TID_OP_INC: 
            case TID_OP_DEC: {
                    int localIdx = getLocal(m_scanner->next(1).lexeme);
                    if (token.tid == TID_OP_INC) m_builder->incLocal(localIdx);
                    else m_builder->decLocal(localIdx);
                    m_builder->loadLocal(localIdx);
                } break;
            default: ASSERT(0); break;
        }
    }
    void _expr_led() {
        Token opToken = m_scanner->next(1);
        switch (opToken.tid) {
            case TID_OP_AND: case TID_OP_OR: {
                    x86Label label_end;
                    m_builder->dup();
                    if (opToken.tid == TID_OP_AND) m_builder->falseJmp(&label_end);
                    else m_builder->trueJmp(&label_end);
                    m_builder->pop(1);
                    _expr(getOperatorRBP(opToken.tid));
                    m_builder->markLabel(&label_end);
                } break;
            case TID_OP_LESS: case TID_OP_LESSEQ: case TID_OP_GREATER: case TID_OP_GREATEREQ: case TID_OP_EQUAL: case TID_OP_NEQUAL:
                _expr(getOperatorRBP(opToken.tid));
                m_builder->cmp(opToken.tid);
                break;
            case TID_OP_ADD: case TID_OP_SUB: case TID_OP_MUL: case TID_OP_DIV: case TID_OP_MOD:
                _expr(getOperatorRBP(opToken.tid));
                m_builder->doArithmeticOp(opToken.tid);
                break;
            default: ASSERT(0); break;
        }
    }
    void _expr_call(const Token &funcToken) {
        ASSERT(m_scanner->next(1).tid == TID_LP);
        int callID = m_builder->beginCall();
        int paramCount = 0;
        while (m_scanner->LA(1).tid != TID_RP) {
            ++paramCount;
            _expr(0);
            if (m_scanner->LA(1).tid == TID_COMMA) m_scanner->next(1);
        }
        ASSERT(m_scanner->next(1).tid == TID_RP);
        m_builder->endCall(funcToken.lexeme, callID, paramCount);
    }
private:
    static int getOperatorLBP(TokenID tid) {
        switch (tid) {
            case TID_RP: case TID_COMMA: case TID_SEMICELON: return 0;
            case TID_OP_AND: case TID_OP_OR: return 10;
            case TID_OP_LESS: case TID_OP_LESSEQ: case TID_OP_GREATER: case TID_OP_GREATEREQ: case TID_OP_EQUAL: case TID_OP_NEQUAL:
                return 20;
            case TID_OP_ADD: case TID_OP_SUB: return 30;
            case TID_OP_MUL: case TID_OP_DIV: case TID_OP_MOD: return 40;
            default: ASSERT(0); return 0;
        }
    }
    static int getOperatorRBP(TokenID tid) {
        switch (tid) {
            case TID_OP_NOT: return 100;
            default: return getOperatorLBP(tid);
        }
    }
private:
    void pushScope() { m_nestedLocals.resize(m_nestedLocals.size() + 1); }
    void popScope() { m_nestedLocals.pop_back(); }
    int declareArg(const string &name, const string &type) {
        ASSERT(m_args.count(name) == 0);
        int idx = -((int)m_args.size() + 1);
        return m_args[name] = idx;
    }
    int declareLocal(const string &name, const string &type) {
        ASSERT(m_nestedLocals.back().count(name) == 0);
        int idx = 0;
        for (int i = 0; i < (int)m_nestedLocals.size(); ++i) idx += (int)m_nestedLocals[i].size();
        ASSERT(idx + 1 <= MAX_LOCAL_COUNT);
        return m_nestedLocals.back()[name] = idx;
    }
    int getLocal(const string &name) {
        map<string, int>::iterator iter;
        for (int i = 0; i < (int)m_nestedLocals.size(); ++i) {
            iter = m_nestedLocals[i].find(name);
            if (iter != m_nestedLocals[i].end()) return iter->second;
        }
        iter = m_args.find(name);
        ASSERT(iter != m_args.end());
        return iter->second;
    }
    x86Label* pushContinueLabel() { m_continueLabels.push_back(new x86Label()); return m_continueLabels.back(); }
    void popContinueLabel() { delete m_continueLabels.back(); m_continueLabels.pop_back();}
    x86Label* getLastContinueLabel() { return m_continueLabels.back(); }
    x86Label* pushBreakLabel(){ m_breakLabels.push_back(new x86Label()); return m_breakLabels.back(); }
    void popBreakLabel(){ delete m_breakLabels.back(); m_breakLabels.pop_back();}
    x86Label* getLastBreakLabel() { return m_breakLabels.back(); }
private:
    x86FunctionBuilder *m_builder;
    Scanner *m_scanner;
    vector<map<string, int> > m_nestedLocals;
    map<string, int> m_args;
    vector<x86Label*> m_continueLabels, m_breakLabels;
};
class FileParser {
public:
    FileParser(x86JITEngine *engine, Scanner *scanner): m_engine(engine), m_scanner(scanner) {}
    void parse() { while (parseFunction()); }
private:
    bool parseFunction() {
        if (m_scanner->LA(1).tid != TID_EOF) {
            x86FunctionBuilder *builder = m_engine->beginBuildFunction();
            FunctionParser(builder, m_scanner).parse();
            m_engine->endBuildFunction(builder);
            return true;
        }
        return false;
    }
private:
    x86JITEngine *m_engine;
    Scanner *m_scanner;
};
//============================== 
static x86JITEngine* g_jitEngine;
static int loadFile(const char *fileName) {
    try {
        g_jitEngine->beginBuild();
        string source = readFile(fileName);
        Scanner scanner(source.c_str());
        FileParser(g_jitEngine, &scanner).parse();
        g_jitEngine->endBuild();
        return 0;
    } catch(const exception &e) {
        puts(e.what());
        return 1;
    }
}
int main(int argc, char *argv[]) {
    x86JITEngine _engine;

    g_jitEngine = &_engine;
    *g_jitEngine->_getFunctionEntry("loadFile") = (char*)loadFile;

    if (argc >= 2) {
        int erro = loadFile(argv[1]);
        return erro ? erro : ((int(*)())g_jitEngine->getFunction("main"))();
    }

    int lineNum = 0;
    for (string line; ; ++lineNum) {
        printf(">>> ");
        if (!getline(cin, line)) break;

        try {
            Scanner scanner(line.c_str());
            bool isFuncDefine = false;
            switch (scanner.LA(1).tid) {
                case TID_TYPE_INT: case TID_TYPE_STRING: case TID_TYPE_VOID: 
                    if (scanner.LA(2).tid == TID_ID && scanner.LA(3).tid == TID_LP) isFuncDefine = true;
                    break;
                default: break;
            }
            if (!isFuncDefine) {
                line = format("int temp_%d(){ return %s; }", lineNum, line.c_str());
                scanner = Scanner(line.c_str());
            }

            g_jitEngine->beginBuild();
            FileParser(g_jitEngine, &scanner).parse();
            g_jitEngine->endBuild();

            if (!isFuncDefine) {
                printf("%d\n", ((int(*)())g_jitEngine->getFunction(format("temp_%d", lineNum)))());
            }

        } catch(const exception &e) {
            puts(e.what());
        } 
    }
}
