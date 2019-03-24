// Copyright (c) 2019 ASMlover. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//  * Redistributions of source code must retain the above copyright
//    notice, this list ofconditions and the following disclaimer.
//
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in
//    the documentation and/or other materialsprovided with the
//    distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
#include <cctype>
#include "lexer.hh"

namespace sage {

Lexer::Lexer(const std::string& source_bytes, const std::string& fname)
  : source_bytes_(source_bytes)
  , fname_(fname) {
}

Token Lexer::next_token(void) {
  skip_whitespace();

  begpos_ = curpos_;
  if (is_end())
    return make_token(TokenKind::TK_EOF);

  char c = advance();
  if (std::isdigit(c))
    return make_numeric();
  if (is_alpha(c))
    return make_identifier();

  switch (c) {
  case '[': return make_token(TokenKind::TK_LSQUARE);
  case ']': return make_token(TokenKind::TK_RSQUARE);
  case '(': return make_token(TokenKind::TK_LPAREN);
  case ')': return make_token(TokenKind::TK_RPAREN);
  case '{': return make_token(TokenKind::TK_LBRACE);
  case '}': return make_token(TokenKind::TK_RBRACE);
  case '.':
    if (match('.')) {
      return make_token(match('.')
          ? TokenKind::TK_PERIODPERIODPERIOD
          : TokenKind::TK_PERIODPERIOD);
    }
    else {
      return make_token(TokenKind::TK_PERIOD);
    }
  case ',': return make_token(TokenKind::TK_COMMA);
  case ':': return make_token(TokenKind::TK_COLON);
  case ';': return make_token(TokenKind::TK_SEMI);
  case '+':
    return make_token(match('=')
        ? TokenKind::TK_PLUSEQUAL : TokenKind::TK_PLUS);
  case '-':
    return make_token(match('=')
        ? TokenKind::TK_MINUSEQUAL : TokenKind::TK_MINUS);
  case '*':
    return make_token(match('=')
        ? TokenKind::TK_STAREQUAL : TokenKind::TK_STAR);
  case '/':
    return make_token(match('=')
        ? TokenKind::TK_SLASHEQUAL : TokenKind::TK_SLASH);
  case '%':
    return make_token(match('=')
        ? TokenKind::TK_PERCENTEQUAL : TokenKind::TK_PERCENT);
  case '<':
    return make_token(match('=')
        ? TokenKind::TK_LESSEQUAL : TokenKind::TK_LESS);
  case '>':
    return make_token(match('>')
        ? TokenKind::TK_GREATEREQUAL : TokenKind::TK_GREATER);
  case '!':
    return make_token(match('=')
        ? TokenKind::TK_EXCLAIMEQUAL : TokenKind::TK_EXCLAIM);
  case '=':
    return make_token(match('=')
        ? TokenKind::TK_EQUALEQUAL : TokenKind::TK_EQUAL);
  case '\n':
    {
      auto tok = make_token(TokenKind::TK_NL);
      ++lineno_;
      return tok;
    }
  case '"': return make_string();
  }

  return error_token("unexpected charactor");
}

bool Lexer::is_alpha(char c) const {
  return std::isalpha(c) || c == '_';
}

bool Lexer::is_alnum(char c) const {
  return std::isalnum(c) || c == '_';
}

std::string Lexer::gen_literal(std::size_t begpos, std::size_t endpos) const {
  return source_bytes_.substr(begpos, endpos - begpos);
}

bool Lexer::is_end(void) const {
  return curpos_ >= source_bytes_.size();
}

char Lexer::advance(void) {
  return source_bytes_[curpos_++];
}

bool Lexer::match(char expected) {
  if (is_end() || source_bytes_[curpos_] != expected)
    return false;

  ++curpos_;
  return true;
}

char Lexer::peek(void) const {
  if (curpos_ >= source_bytes_.size())
    return 0;
  return source_bytes_[curpos_];
}

char Lexer::peek_next(void) const {
  if (curpos_ + 1 >= source_bytes_.size())
    return 0;
  return source_bytes_[curpos_ + 1];
}

Token Lexer::make_token(TokenKind kind) {
  return make_token(kind, gen_literal(begpos_, curpos_));
}

Token Lexer::make_token(TokenKind kind, const std::string& literal) {
  return Token(kind, literal, fname_, lineno_);
}

Token Lexer::error_token(const std::string& message) {
  return Token(TokenKind::TK_ERROR, message, fname_, lineno_);
}

void Lexer::skip_whitespace(void) {
  for (;;) {
    char c = peek();
    switch (c) {
    case ' ':
    case '\r':
    case '\t':
      // ignore whitespaces
      advance(); break;
    case '#': skip_comment(); break;
    default: return;
    }
  }
}

void Lexer::skip_comment(void) {
  while (!is_end() && peek() != '\n')
    advance();
}

Token Lexer::make_string(void) {
  std::string literal;
  while (!is_end() && peek() != '"') {
    char c = peek();
    switch (c) {
    case '\n': ++lineno_; break;
    case '\\':
      switch (peek_next()) {
      case '"': c = '"'; advance(); break;
      case '\\': c = '\\'; advance(); break;
      case '%': c = '%'; advance(); break;
      case '0': c = '\0'; advance(); break;
      case 'a': c = '\a'; advance(); break;
      case 'b': c = '\b'; advance(); break;
      case 'f': c = '\f'; advance(); break;
      case 'n': c = '\n'; advance(); break;
      case 'r': c = '\r'; advance(); break;
      case 't': c = '\t'; advance(); break;
      case 'v': c = '\v'; advance(); break;
      }
      break;
    }
    literal.push_back(c);
    advance();
  }

  // unterminated string
  if (is_end())
    return error_token("unterminated string");

  // the closing "
  advance();

  // trim the surrounding string
  return make_token(TokenKind::TK_STRINGLITERAL, literal);
}

Token Lexer::make_numeric(void) {
  while (std::isdigit(peek()))
    advance();

  TokenKind kind{TokenKind::TK_INTEGERCONST};
  if (peek() == '.' && std::isdigit(peek_next())) {
    advance();
    while (std::isdigit(peek()))
      advance();

    kind = TokenKind::TK_DECIMALCONST;
  }

  if (is_alpha(peek()))
    return error_token("invalid numeric or identifier");
  return make_token(kind);
}

Token Lexer::make_identifier(void) {
  while (is_alnum(peek()))
    advance();

  auto literal = gen_literal(begpos_, curpos_);
  return make_token(get_keyword_kind(literal.c_str()), literal);
}

}
