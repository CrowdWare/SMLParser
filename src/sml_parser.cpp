#include "sml_parser.h"

#include <cctype>
#include <cstdlib>
#include <sstream>

namespace sml {

static std::string FormatMessage(const std::string& message, const Span& span) {
    std::ostringstream os;
    os << message << " at line " << span.line << ", col " << span.col;
    return os.str();
}

SmlParseException::SmlParseException(const std::string& message, const Span& span)
    : std::runtime_error(FormatMessage(message, span))
    , span(span) {
}

PropertyValue PropertyValue::FromInt(int v) {
    PropertyValue pv;
    pv.type = Int;
    pv.int_value = v;
    return pv;
}

PropertyValue PropertyValue::FromFloat(float v) {
    PropertyValue pv;
    pv.type = Float;
    pv.float_value = v;
    return pv;
}

PropertyValue PropertyValue::FromBool(bool v) {
    PropertyValue pv;
    pv.type = Boolean;
    pv.bool_value = v;
    return pv;
}

PropertyValue PropertyValue::FromString(const std::string& v) {
    PropertyValue pv;
    pv.type = String;
    pv.string_value = v;
    return pv;
}

PropertyValue PropertyValue::FromVec2i(int x, int y) {
    PropertyValue pv;
    pv.type = Vec2iType;
    pv.vec2i_value = Vec2i{x, y};
    return pv;
}

PropertyValue PropertyValue::FromVec3i(int x, int y, int z) {
    PropertyValue pv;
    pv.type = Vec3iType;
    pv.vec3i_value = Vec3i{x, y, z};
    return pv;
}

PropertyValue PropertyValue::FromEnum(const std::string& v) {
    PropertyValue pv;
    pv.type = EnumType;
    pv.string_value = v;
    return pv;
}

SmlLexer::SmlLexer(const std::string& input)
    : input_(input), i_(0), line_(1), col_(1) {
}

char SmlLexer::peek(int offset) const {
    int index = i_ + offset;
    if (index < 0 || index >= static_cast<int>(input_.size()))
        return '\0';
    return input_[index];
}

void SmlLexer::advance(int n) {
    for (int step = 0; step < n; ++step) {
        if (i_ >= static_cast<int>(input_.size()))
            return;
        char c = input_[i_];
        ++i_;
        if (c == '\n') {
            ++line_;
            col_ = 1;
        } else {
            ++col_;
        }
    }
}

Span SmlLexer::spanStart() const {
    Span s;
    s.index = i_;
    s.line = line_;
    s.col = col_;
    return s;
}

Token SmlLexer::makeToken(TokenType type, const Span& start, const std::string& text) const {
    Token t;
    t.type = type;
    t.text = text;
    t.start = start;
    t.end = Span{i_, line_, col_};
    return t;
}

Token SmlLexer::next() {
    if (i_ >= static_cast<int>(input_.size()))
        return Token{TokenType::Eof, "", Span{i_, line_, col_}, Span{i_, line_, col_}};

    if (std::isspace(static_cast<unsigned char>(peek()))) {
        Span start = spanStart();
        std::string text;
        while (std::isspace(static_cast<unsigned char>(peek()))) {
            text.push_back(peek());
            advance();
        }
        return makeToken(TokenType::Ws, start, text);
    }

    if (peek() == '/' && peek(1) == '/') {
        Span start = spanStart();
        std::string text;
        while (peek() != '\0' && peek() != '\n') {
            text.push_back(peek());
            advance();
        }
        return makeToken(TokenType::LineComment, start, text);
    }

    if (peek() == '/' && peek(1) == '*') {
        Span start = spanStart();
        advance(2);
        std::string text("/*");
        while (true) {
            char c = peek();
            if (c == '\0')
                break;
            if (c == '*' && peek(1) == '/') {
                text.append("*/");
                advance(2);
                break;
            }
            text.push_back(c);
            advance();
        }
        return makeToken(TokenType::BlockComment, start, text);
    }

    char c = peek();
    if (c == '{') {
        Span s = spanStart();
        advance();
        return makeToken(TokenType::LBrace, s, "{");
    }
    if (c == '}') {
        Span s = spanStart();
        advance();
        return makeToken(TokenType::RBrace, s, "}");
    }
    if (c == ':') {
        Span s = spanStart();
        advance();
        return makeToken(TokenType::Colon, s, ":");
    }
    if (c == ',') {
        Span s = spanStart();
        advance();
        return makeToken(TokenType::Comma, s, ",");
    }
    if (c == '"')
        return lexString();
    if (c == '\0')
        return Token{TokenType::Eof, "", Span{i_, line_, col_}, Span{i_, line_, col_}};

    if (std::isalpha(static_cast<unsigned char>(c)) || c == '_')
        return lexIdentOrBool();
    if (std::isdigit(static_cast<unsigned char>(c)))
        return lexNumber();

    throw SmlParseException(std::string("Unexpected character '") + c + "'", Span{i_, line_, col_});
}

Token SmlLexer::lexString() {
    Span start = spanStart();
    std::string text;
    char quote = peek();
    advance();
    while (true) {
        char c = peek();
        if (c == '\0')
            throw SmlParseException("Unterminated string literal", Span{i_, line_, col_});
        if (c == quote) {
            advance();
            break;
        }
        text.push_back(c);
        advance();
    }
    return Token{TokenType::String, text, start, Span{i_, line_, col_}};
}

Token SmlLexer::lexIdentOrBool() {
    Span start = spanStart();
    std::string text;
    bool first = true;
    while (true) {
        char c = peek();
        if (c == '\0')
            break;
        bool ok = first ? (std::isalpha(static_cast<unsigned char>(c)) || c == '_')
                        : (std::isalnum(static_cast<unsigned char>(c)) || c == '_');
        if (!ok)
            break;
        text.push_back(c);
        advance();
        first = false;
    }
    if (text == "true" || text == "false")
        return Token{TokenType::Bool, text, start, Span{i_, line_, col_}};
    return Token{TokenType::Ident, text, start, Span{i_, line_, col_}};
}

Token SmlLexer::lexNumber() {
    Span start = spanStart();
    std::string text;
    while (std::isdigit(static_cast<unsigned char>(peek()))) {
        text.push_back(peek());
        advance();
    }
    if (peek() == '.' && std::isdigit(static_cast<unsigned char>(peek(1)))) {
        text.push_back('.');
        advance();
        while (std::isdigit(static_cast<unsigned char>(peek()))) {
            text.push_back(peek());
            advance();
        }
        return Token{TokenType::Float, text, start, Span{i_, line_, col_}};
    }
    return Token{TokenType::Int, text, start, Span{i_, line_, col_}};
}

SmlSaxParser::SmlSaxParser(const std::string& text)
    : lexer_(text), lookahead_(lexer_.next()) {
}

void SmlSaxParser::registerEnumValue(const std::string& property, const std::string& value) {
    enums_[property].insert(value);
}

void SmlSaxParser::registerEnumValues(const std::string& property, const std::vector<std::string>& values) {
    std::set<std::string>& set_ref = enums_[property];
    for (size_t i = 0; i < values.size(); ++i)
        set_ref.insert(values[i]);
}

void SmlSaxParser::parse(SmlHandler& handler) {
    skipIgnorables();
    while (lookahead_.type != TokenType::Eof) {
        parseElement(handler);
        skipIgnorables();
    }
}

void SmlSaxParser::parseElement(SmlHandler& handler) {
    std::string name = expect(TokenType::Ident).text;
    skipIgnorables();
    expect(TokenType::LBrace);
    handler.startElement(name);
    skipIgnorables();

    while (lookahead_.type != TokenType::RBrace && lookahead_.type != TokenType::Eof) {
        if (lookahead_.type != TokenType::Ident)
            throw SmlParseException("Expected property or element", lookahead_.start);
        Token ident = consume();
        skipIgnorables();
        if (lookahead_.type == TokenType::Colon) {
            consume();
            skipIgnorables();
            PropertyValue value = parseValue(ident.text);
            handler.onProperty(ident.text, value);
            skipIgnorables();
        } else {
            if (lookahead_.type != TokenType::LBrace) {
                throw SmlParseException("Expected '{' after nested element name '" + ident.text + "'", lookahead_.start);
            }
            consume();
            handler.startElement(ident.text);
            skipIgnorables();
            parseElementBody(handler);
            expect(TokenType::RBrace);
            handler.endElement(ident.text);
            skipIgnorables();
        }
    }

    expect(TokenType::RBrace);
    handler.endElement(name);
}

void SmlSaxParser::parseElementBody(SmlHandler& handler) {
    while (lookahead_.type != TokenType::RBrace && lookahead_.type != TokenType::Eof) {
        if (lookahead_.type != TokenType::Ident)
            throw SmlParseException("Expected property or element", lookahead_.start);
        Token ident = consume();
        skipIgnorables();
        if (lookahead_.type == TokenType::Colon) {
            consume();
            skipIgnorables();
            PropertyValue value = parseValue(ident.text);
            handler.onProperty(ident.text, value);
            skipIgnorables();
        } else {
            if (lookahead_.type != TokenType::LBrace) {
                throw SmlParseException("Expected '{' after nested element name '" + ident.text + "'", lookahead_.start);
            }
            consume();
            handler.startElement(ident.text);
            skipIgnorables();
            parseElementBody(handler);
            expect(TokenType::RBrace);
            handler.endElement(ident.text);
            skipIgnorables();
        }
    }
}

PropertyValue SmlSaxParser::parseValue(const std::string& property) {
    if (lookahead_.type == TokenType::String)
        return PropertyValue::FromString(consume().text);
    if (lookahead_.type == TokenType::Float)
        return PropertyValue::FromFloat(static_cast<float>(std::atof(consume().text.c_str())));
    if (lookahead_.type == TokenType::Bool)
        return PropertyValue::FromBool(consume().text == "true");
    if (lookahead_.type == TokenType::Int) {
        int first = std::atoi(consume().text.c_str());
        skipIgnorables();
        if (lookahead_.type != TokenType::Comma)
            return PropertyValue::FromInt(first);

        consume();
        skipIgnorables();
        if (lookahead_.type != TokenType::Int)
            throw SmlParseException("Expected integer component after ','", lookahead_.start);
        int second = std::atoi(consume().text.c_str());
        skipIgnorables();

        if (lookahead_.type == TokenType::Comma) {
            consume();
            skipIgnorables();
            if (lookahead_.type != TokenType::Int)
                throw SmlParseException("Expected integer component after ','", lookahead_.start);
            int third = std::atoi(consume().text.c_str());
            return PropertyValue::FromVec3i(first, second, third);
        }
        return PropertyValue::FromVec2i(first, second);
    }
    if (lookahead_.type == TokenType::Ident) {
        Token value_token = consume();
        if (!isEnumValueAllowed(property, value_token.text)) {
            throw SmlParseException("Unknown enum value '" + value_token.text + "' for property '" + property + "'", value_token.start);
        }
        return PropertyValue::FromEnum(value_token.text);
    }

    throw SmlParseException("Expected value", lookahead_.start);
}

Token SmlSaxParser::expect(TokenType type) {
    if (lookahead_.type != type) {
        throw SmlParseException("Expected token type mismatch", lookahead_.start);
    }
    return consume();
}

Token SmlSaxParser::consume() {
    Token t = lookahead_;
    lookahead_ = lexer_.next();
    return t;
}

void SmlSaxParser::skipIgnorables() {
    while (lookahead_.type == TokenType::Ws ||
           lookahead_.type == TokenType::LineComment ||
           lookahead_.type == TokenType::BlockComment) {
        lookahead_ = lexer_.next();
    }
}

bool SmlSaxParser::isEnumValueAllowed(const std::string& property, const std::string& value) const {
    std::map<std::string, std::set<std::string> >::const_iterator it = enums_.find(property);
    if (it == enums_.end())
        return false;
    return it->second.find(value) != it->second.end();
}

} // namespace sml
