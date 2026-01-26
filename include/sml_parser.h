#ifndef SML_PARSER_H
#define SML_PARSER_H

#include <stdexcept>
#include <string>
#include <map>
#include <set>
#include <vector>

namespace sml {

enum class TokenType {
    Ident,
    LBrace,
    RBrace,
    Colon,
    String,
    Int,
    Float,
    Bool,
    LineComment,
    BlockComment,
    Ws,
    Comma,
    Eof
};

struct Span {
    int index;
    int line;
    int col;
};

struct Token {
    TokenType type;
    std::string text;
    Span start;
    Span end;
};

class SmlParseException : public std::runtime_error {
public:
    SmlParseException(const std::string& message, const Span& span);
    const Span span;
};

struct Vec2i {
    int x;
    int y;
};

struct Vec3i {
    int x;
    int y;
    int z;
};

struct PropertyValue {
    enum Type { Int, Float, Boolean, String, Vec2iType, Vec3iType, EnumType } type;
    int int_value;
    float float_value;
    bool bool_value;
    std::string string_value;
    Vec2i vec2i_value;
    Vec3i vec3i_value;

    static PropertyValue FromInt(int v);
    static PropertyValue FromFloat(float v);
    static PropertyValue FromBool(bool v);
    static PropertyValue FromString(const std::string& v);
    static PropertyValue FromVec2i(int x, int y);
    static PropertyValue FromVec3i(int x, int y, int z);
    static PropertyValue FromEnum(const std::string& v);
};

class SmlHandler {
public:
    virtual ~SmlHandler() {}
    virtual void startElement(const std::string& name) = 0;
    virtual void onProperty(const std::string& name, const PropertyValue& value) = 0;
    virtual void endElement(const std::string& name) = 0;
};

class SmlLexer {
public:
    explicit SmlLexer(const std::string& input);
    Token next();

private:
    char peek(int offset = 0) const;
    void advance(int n = 1);
    Span spanStart() const;
    Token makeToken(TokenType type, const Span& start, const std::string& text) const;
    Token lexString();
    Token lexIdentOrBool();
    Token lexNumber();

    std::string input_;
    int i_;
    int line_;
    int col_;
};

class SmlSaxParser {
public:
    explicit SmlSaxParser(const std::string& text);
    void parse(SmlHandler& handler);
    void registerEnumValue(const std::string& property, const std::string& value);
    void registerEnumValues(const std::string& property, const std::vector<std::string>& values);

private:
    void parseElement(SmlHandler& handler);
    void parseElementBody(SmlHandler& handler);
    PropertyValue parseValue(const std::string& property);
    Token expect(TokenType type);
    Token consume();
    void skipIgnorables();
    bool isEnumValueAllowed(const std::string& property, const std::string& value) const;

    SmlLexer lexer_;
    Token lookahead_;
    std::map<std::string, std::set<std::string> > enums_;
};

} // namespace sml

#endif
