#pragma once

// Enum for token types
enum class TokenType {
    Keyword,
    Identifier,
    Operator,
    Datatype,
    Number,
    String,
    Punctuation,
    Whitespace,
    Unknown
};