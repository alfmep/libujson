/*
 * Copyright (C) 2022-2024 Dan Arrhenius <dan@ultramarin.se>
 *
 * This file is part of ujson.
 *
 * ujson is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include <ujson/schema/jvocabulary_validation.hpp>
#include <ujson/jschema.hpp>
#include <ujson/invalid_schema.hpp>
#include <ujson/utils.hpp>
#include <regex>
#if ! (UJSON_HAVE_GMPXX)
#include <cmath>
#endif

#define DEVEL_DEBUG 0

#if (DEVEL_DEBUG)
#include <iostream>
using std::cin;
using std::cout;
using std::cerr;
using std::endl;
#define PREFIX __FUNCTION__ << ',' << __LINE__ << ": "
#endif


namespace ujson::schema {

    const jvocabulary_validation::keywords_t jvocabulary_validation::keywords = {{
            //
            // Any instance type
            //
            {"type", std::make_tuple(
                        j_invalid,
                        &jvocabulary_validation::load_type,
                        &jvocabulary_validation::validate_type)},
            {"enum", std::make_tuple(
                        j_invalid,
                        &jvocabulary_validation::load_enum,
                        &jvocabulary_validation::validate_enum)},
            {"const", std::make_tuple(
                        j_invalid,
                        nullptr,
                        &jvocabulary_validation::validate_const)},
            //
            // Numeric instances
            //
            {"multipleOf", std::make_tuple(
                        j_number,
                        &jvocabulary_validation::load_multipleOf,
                        &jvocabulary_validation::validate_multipleOf)},
            {"maximum", std::make_tuple(
                        j_number,
                        &jvocabulary_validation::load_kw_type_is_number,
                        &jvocabulary_validation::validate_maximum)},
            {"exclusiveMaximum", std::make_tuple(
                        j_number,
                        &jvocabulary_validation::load_kw_type_is_number,
                        &jvocabulary_validation::validate_exclusiveMaximum)},
            {"minimum", std::make_tuple(
                        j_number,
                        &jvocabulary_validation::load_kw_type_is_number,
                        &jvocabulary_validation::validate_minimum)},
            {"exclusiveMinimum", std::make_tuple(
                        j_number,
                        &jvocabulary_validation::load_kw_type_is_number,
                        &jvocabulary_validation::validate_exclusiveMinimum)},
            //
            // String instances
            //
            {"maxLength", std::make_tuple(
                        j_string,
                        &jvocabulary_validation::load_kw_type_is_positive_number,
                        &jvocabulary_validation::validate_maxLength)},
            {"minLength", std::make_tuple(
                        j_string,
                        &jvocabulary_validation::load_kw_type_is_positive_number,
                        &jvocabulary_validation::validate_minLength)},
            {"pattern", std::make_tuple(
                        j_string,
                        &jvocabulary_validation::load_pattern,
                        &jvocabulary_validation::validate_pattern)},
            //
            // Array instances
            //
            {"maxItems", std::make_tuple(
                        j_array,
                        &jvocabulary_validation::load_kw_type_is_positive_number,
                        &jvocabulary_validation::validate_maxItems)},
            {"minItems", std::make_tuple(
                        j_array,
                        &jvocabulary_validation::load_kw_type_is_positive_number,
                        &jvocabulary_validation::validate_minItems)},
            {"uniqueItems", std::make_tuple(
                        j_array,
                        &jvocabulary_validation::load_uniqueItems,
                        &jvocabulary_validation::validate_uniqueItems)},
            {"maxContains", std::make_tuple(
                        j_array,
                        &jvocabulary_validation::load_kw_type_is_positive_number,
                        &jvocabulary_validation::validate_maxContains)},
            {"minContains", std::make_tuple(
                        j_array,
                        &jvocabulary_validation::load_kw_type_is_positive_number,
                        &jvocabulary_validation::validate_minContains)},
            //
            // Object instances
            //
            {"maxProperties", std::make_tuple(
                        j_object,
                        &jvocabulary_validation::load_kw_type_is_positive_number,
                        &jvocabulary_validation::validate_maxProperties)},
            {"minProperties", std::make_tuple(
                        j_object,
                        &jvocabulary_validation::load_kw_type_is_positive_number,
                        &jvocabulary_validation::validate_minProperties)},
            {"required", std::make_tuple(
                        j_object,
                        &jvocabulary_validation::load_required,
                        &jvocabulary_validation::validate_required)},
            {"dependentRequired", std::make_tuple(
                        j_object,
                        &jvocabulary_validation::load_dependentRequired,
                        &jvocabulary_validation::validate_dependentRequired)},
        }};


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
#if UJSON_HAVE_GMPXX
    static bool is_integer (const mpf_class& value)
    {
        return mpf_integer_p(value.get_mpf_t()) != 0;
    }
#else
    static bool is_integer (const double value)
    {
        double i;
        // We don't count NaN or infinity as an integer
        if (!std::isinf(value) && !std::isnan(value))
            return modf(value, &i) == (double)0.0;
        else
            return false;
    }
#endif


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    static bool is_integer (jvalue& instance)
    {
#if UJSON_HAVE_GMPXX
        return instance.type()==j_number && is_integer(instance.mpf());
#else
        return instance.type()==j_number && is_integer(instance.num());
#endif
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    static bool is_non_negative_integer (jvalue& value)
    {
        if (value.type() != j_number)
            return false;
#if UJSON_HAVE_GMPXX
        return is_integer(value.mpf())  &&  value.mpf() >= 0;
#else
        return is_integer(value.num())  &&  value.num() >= 0;
#endif
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jvocabulary_validation::jvocabulary_validation (jschema& schema_arg)
        : jvocabulary (schema_arg, "https://json-schema.org/draft/2020-12/meta/validation")
    {
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void jvocabulary_validation::load (jvalue& schema, jvalue& load_ctx)
    {
        for (auto& member : schema.obj()) {
            const std::string& schema_keyword = member.first;
            ujson::jvalue& schema_value = member.second;

            // Find a loader for this keyword
            auto entry = keywords.find (schema_keyword);
            if (entry != keywords.end()) {
                auto kw_loader = std::get<1> (entry->second);
                if (kw_loader) {
                    push_load_ctx_path (load_ctx, schema_keyword);
                    (this->*kw_loader) (schema_keyword, schema_value, load_ctx);
                    pop_load_ctx_path (load_ctx);
                }
            }
        }
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void jvocabulary_validation::load_kw_type_is_number (const std::string& keyword,
                                                         jvalue& schema_value,
                                                         jvalue& load_ctx)
    {
        if (schema_value.type() != j_number) {
            std::string what = "Schema keyword '";
            what.append (keyword);
            what.append ("' not a number.");
            throw invalid_schema (what);
        }
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void jvocabulary_validation::load_kw_type_is_positive_number (const std::string& keyword,
                                                                  jvalue& schema_value,
                                                                  jvalue& load_ctx)
    {
        if (!is_non_negative_integer(schema_value)) {
            std::string what = "Schema keyword '";
            what.append (keyword);
            what.append ("' not a non-negative integer.");
            throw invalid_schema (what);
        }
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void jvocabulary_validation::load_single_type (jvalue& schema_value, jvalue& load_ctx)
    {
        if (schema_value.type() == j_string) {
            auto& type_name = schema_value.str ();
            auto t = str_to_jtype (type_name);
            if (t == j_invalid) {
                if (type_name != "integer") {
                    throw invalid_schema ("Invalid type name for schema keyword 'type'.");
                }
            }
        }else{
            // 6.1.1. The value of this keyword MUST be either a string or an array.
            throw invalid_schema ("Schema keyword 'type' not a string or array of strings.");
        }
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void jvocabulary_validation::load_type (const std::string& keyword,
                                            jvalue& schema_value,
                                            jvalue& load_ctx)
    {
        if (schema_value.type() == j_array) {
            auto& a = schema_value.array ();
            for (unsigned i=0; i<a.size(); ++i) {
                push_load_ctx_path (load_ctx, std::to_string(i));
                load_single_type (a[i], load_ctx);
                pop_load_ctx_path (load_ctx);
            }
        }
        else {
            load_single_type (schema_value, load_ctx);
        }
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void jvocabulary_validation::load_enum (const std::string& keyword,
                                            jvalue& schema_value,
                                            jvalue& load_ctx)
    {
        // 6.1.2. The value of this keyword MUST be an array.
        if (schema_value.type() != j_array) {
            throw invalid_schema ("Schema keyword 'enum' not an array.");
        }
        // 6.1.2. This array SHOULD have at least one element.
        if (schema_value.array().empty()) {
            throw invalid_schema ("Schema keyword 'enum' is an empty array.");
        }
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void jvocabulary_validation::load_multipleOf (const std::string& keyword,
                                                  jvalue& schema_value,
                                                  jvalue& load_ctx)
    {
        // 6.2.1. The value of "multipleOf" MUST be a number, strictly greater than 0.
        if (schema_value.type() != j_number)
            throw invalid_schema ("Schema keyword 'multipleOf' not a number.");

#if UJSON_HAVE_GMPXX
        auto& value_num = schema_value.mpf ();
#else
        auto value_num = schema_value.num ();
#endif

        // 6.2.1. The value of "multipleOf" MUST be a number, strictly greater than 0.
        if (value_num <= 0)
            throw invalid_schema ("Schema keyword 'multipleOf' not greater than 0.");
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void jvocabulary_validation::load_pattern (const std::string& keyword,
                                               jvalue& schema_value,
                                               jvalue& load_ctx)
    {
        try {
            // 6.3.3. The value of this keyword MUST be a string.
            if (schema_value.type() != j_string)
                throw invalid_schema ("Schema keyword 'pattern' not a string.");

            std::regex re (schema_value.str(), std::regex::ECMAScript);
        }
        catch (std::regex_error& re) {
            throw invalid_schema ("Schema keyword 'pattern' not a valid regular expression.");
        }
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void jvocabulary_validation::load_uniqueItems (const std::string& keyword,
                                                   jvalue& schema_value,
                                                   jvalue& load_ctx)
    {
        // 6.4.3. The value of this keyword MUST be a boolean
        if (schema_value.type() != j_bool)
            throw invalid_schema ("Schema keyword 'uniqueItems' not a bool.");
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void jvocabulary_validation::load_required (const std::string& keyword,
                                                jvalue& schema_value,
                                                jvalue& load_ctx)
    {
        // 6.5.3. The value of this keyword MUST be an array.
        // Elements of this array, if any, MUST be strings, and MUST be unique.

        if (schema_value.type() == j_array) {
            auto& a = schema_value.array ();
            for (unsigned i=0; i<a.size(); ++i) {
                if (a[i].type() != j_string) {
                    push_load_ctx_path (load_ctx, std::to_string(i));
                    throw invalid_schema ("Schema keyword 'required' not an array of strings.");
                }
            }
        }else{
            throw invalid_schema ("Schema keyword 'required' not an array of strings.");
        }
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void jvocabulary_validation::load_dependentRequired (const std::string& keyword,
                                                         jvalue& schema_value,
                                                         jvalue& load_ctx)
    {
        // 6.5.4. The value of this keyword MUST be an object. Properties in this
        // object, if any, MUST be arrays. Elements in each array, if any, MUST
        // be strings, and MUST be unique.
        static constexpr const char* const err_msg =
            "Schema keyword 'dependentRequired' not an object with arrays of strings.";

        if (schema_value.type() == j_object) {
            for (auto& member : schema_value.obj()) {
                push_load_ctx_path (load_ctx, member.first);

                if (member.second.type() != j_array)
                    throw invalid_schema (err_msg);

                auto& a = member.second.array ();
                for (unsigned i=0; i<a.size(); ++i) {
                    if (a[i].type() != j_string) {
                        push_load_ctx_path (load_ctx, std::to_string(i));
                        throw invalid_schema (err_msg);
                    }
                }
                pop_load_ctx_path (load_ctx);
            }
        }else{
            throw invalid_schema (err_msg);
        }
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    bool jvocabulary_validation::validate (validation_context& ctx,
                                           jvalue& schema,
                                           jvalue& instance,
                                           const bool quit_on_first_error)
    {
        bool valid = true;

        for (auto& member : schema.obj()) {

            const std::string& schema_keyword = member.first;
            ujson::jvalue& schema_value = member.second;

            // Find a validator for this keyword
            auto entry = keywords.find (schema_keyword);
            if (entry == keywords.end())
                continue;

            // Check if the keyword handles this type of instance
            auto keyword_type = std::get<0> (entry->second);
            if (keyword_type==instance.type() || keyword_type==j_invalid) {
                std::string error_msg;
                auto kw_validator = std::get<2> (entry->second);

                ctx.push_schema_path (schema_keyword);
                if ((this->*kw_validator)(ctx, schema, schema_value, instance, error_msg)) {
                    ctx.append_sub_ou ();
                }else{
                    if (error_msg.empty() == false) {
                        valid = false;
                        ctx.set_valid (false);
                        ctx.append_error (error_msg);
                    }else{
                        // A non-valid return with an empty error message means it wasn't evaluated at all
                    }
                }
                ctx.pop_schema_path ();
            }
        }

        return valid;
    }


    //--------------------------------------------------------------------------
    // Any instance type
    //--------------------------------------------------------------------------
    bool jvocabulary_validation::validate_type_impl (validation_context& ctx,
                                                     jvalue& schema,
                                                     std::string& type_name,
                                                     jvalue& instance,
                                                     std::string& error_msg,
                                                     bool set_error_msg)
    {
        bool valid = true;

        auto wanted_type = str_to_jtype (type_name);
        if (wanted_type == j_invalid)
            valid = is_integer (instance);
        else
            valid = instance.type() == wanted_type;

        if (!valid && set_error_msg) {
            error_msg = "Value is not of type ";
            error_msg.append (type_name);
            error_msg.push_back ('.');
        }

        return valid;
    }


    //--------------------------------------------------------------------------
    // Any instance type
    //--------------------------------------------------------------------------
    bool jvocabulary_validation::validate_type (validation_context& ctx,
                                                jvalue& schema,
                                                jvalue& schema_value,
                                                jvalue& instance,
                                                std::string& error_msg)
    {
        if (schema_value.type() == j_string) {
            return validate_type_impl (ctx, schema, schema_value.str(), instance, error_msg);
        }
        else if (schema_value.type() == j_array) {
            for (auto& sv : schema_value.array()) {
                if (validate_type_impl(ctx, schema, sv.str(), instance, error_msg, false))
                    return true;
            }
        }

        error_msg = "Value is not of requested type.";
        return false;
    }


    //--------------------------------------------------------------------------
    // Any instance type
    //--------------------------------------------------------------------------
    bool jvocabulary_validation::validate_enum (validation_context& ctx,
                                                jvalue& schema,
                                                jvalue& schema_value,
                                                jvalue& instance,
                                                std::string& error_msg)
    {
        for (auto& element : schema_value.array()) {
            if (element == instance)
                return true;
        }

        error_msg = "Value not one of the allowed values in enum.";
        return false;
    }


    //--------------------------------------------------------------------------
    // Any instance type
    //--------------------------------------------------------------------------
    bool jvocabulary_validation::validate_const (validation_context& ctx,
                                                 jvalue& schema,
                                                 jvalue& schema_value,
                                                 jvalue& instance,
                                                 std::string& error_msg)
    {
        bool valid = schema_value == instance;
        if (!valid)
            error_msg = "Value not same as const value.";
        return valid;
    }


    //--------------------------------------------------------------------------
    // Numeric instances
    //--------------------------------------------------------------------------
    bool jvocabulary_validation::validate_multipleOf (validation_context& ctx,
                                                      jvalue& schema,
                                                      jvalue& schema_value,
                                                      jvalue& instance,
                                                      std::string& error_msg)
    {
#if UJSON_HAVE_GMPXX
        auto& vdata_num = instance.mpf ();
        auto& value_num = schema_value.mpf ();
#else
        auto vdata_num = instance.num ();
        auto value_num = schema_value.num ();
#endif
        bool valid = is_integer (vdata_num / value_num);
        if (!valid)
            error_msg = std::string("Number not a multiple of ") + schema_value.describe();
        return valid;
    }


    //--------------------------------------------------------------------------
    // Numeric instances
    //--------------------------------------------------------------------------
    bool jvocabulary_validation::validate_maximum (validation_context& ctx,
                                                   jvalue& schema,
                                                   jvalue& schema_value,
                                                   jvalue& instance,
                                                   std::string& error_msg)
    {
#if UJSON_HAVE_GMPXX
        bool valid = instance.mpf() <= schema_value.mpf();
#else
        bool valid = instance.num() <= schema_value.num ();
#endif
        if (!valid)
            error_msg = std::string("Number greater than ") + schema_value.describe();
        return valid;
    }


    //--------------------------------------------------------------------------
    // Numeric instances
    //--------------------------------------------------------------------------
    bool jvocabulary_validation::validate_exclusiveMaximum (validation_context& ctx,
                                                            jvalue& schema,
                                                            jvalue& schema_value,
                                                            jvalue& instance,
                                                            std::string& error_msg)
    {
#if UJSON_HAVE_GMPXX
        bool valid = instance.mpf() < schema_value.mpf();
#else
        bool valid = instance.num() < schema_value.num ();
#endif
        if (!valid)
            error_msg = std::string("Number greater than or equal to ") + schema_value.describe();
        return valid;
    }


    //--------------------------------------------------------------------------
    // Numeric instances
    //--------------------------------------------------------------------------
    bool jvocabulary_validation::validate_minimum (validation_context& ctx,
                                                   jvalue& schema,
                                                   jvalue& schema_value,
                                                   jvalue& instance,
                                                   std::string& error_msg)
    {
#if UJSON_HAVE_GMPXX
        bool valid = instance.mpf() >= schema_value.mpf();
#else
        bool valid = instance.num() >= schema_value.num ();
#endif
        if (!valid)
            error_msg = std::string("Number less than ") + schema_value.describe();
        return valid;
    }


    //--------------------------------------------------------------------------
    // Numeric instances
    //--------------------------------------------------------------------------
    bool jvocabulary_validation::validate_exclusiveMinimum (validation_context& ctx,
                                                            jvalue& schema,
                                                            jvalue& schema_value,
                                                            jvalue& instance,
                                                            std::string& error_msg)
    {
#if UJSON_HAVE_GMPXX
        bool valid = instance.mpf() > schema_value.mpf();
#else
        bool valid = instance.num() > schema_value.num ();
#endif
        if (!valid)
            error_msg = std::string("Number less than or equal to ") + schema_value.describe();
        return valid;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    static size_t simple_utf8_len (const std::string& str)
    {
        size_t retval = 0;
        for (size_t i=0; i<str.length(); ++i)
            if ((str[i] & 0xc0) != 0x80)
                ++retval;
        return retval;
    }


    //--------------------------------------------------------------------------
    // String instances
    //--------------------------------------------------------------------------
    bool jvocabulary_validation::validate_maxLength (validation_context& ctx,
                                                     jvalue& schema,
                                                     jvalue& schema_value,
                                                     jvalue& instance,
                                                     std::string& error_msg)
    {
#if UJSON_HAVE_GMPXX
        size_t max_len = (size_t) schema_value.mpf().get_ui ();
#else
        size_t max_len = (size_t) schema_value.num ();
#endif
        bool valid = simple_utf8_len(instance.str()) <= max_len;
        if (!valid)
            error_msg = "String too long.";
        return valid;
    }


    //--------------------------------------------------------------------------
    // String instances
    //--------------------------------------------------------------------------
    bool jvocabulary_validation::validate_minLength (validation_context& ctx,
                                                     jvalue& schema,
                                                     jvalue& schema_value,
                                                     jvalue& instance,
                                                     std::string& error_msg)
    {
#if UJSON_HAVE_GMPXX
        size_t min_len = (size_t) schema_value.mpf().get_ui ();
#else
        size_t min_len = (size_t) schema_value.num ();
#endif
        bool valid = simple_utf8_len(instance.str()) >= min_len;
        if (!valid)
            error_msg = "String too short.";
        return valid;
    }


    //--------------------------------------------------------------------------
    // String instances
    //--------------------------------------------------------------------------
    bool jvocabulary_validation::validate_pattern (validation_context& ctx,
                                                   jvalue& schema,
                                                   jvalue& schema_value,
                                                   jvalue& instance,
                                                   std::string& error_msg)
    {
        bool valid = true;
        std::regex re (schema_value.str(), std::regex::ECMAScript);
        std::cmatch cm;
        valid = (bool) std::regex_search (instance.str().c_str(), cm, re);
        if (!valid)
            error_msg = "String failed regular expression check.";

        return valid;
    }


    //--------------------------------------------------------------------------
    // Array instances
    //--------------------------------------------------------------------------
    bool jvocabulary_validation::validate_maxItems (validation_context& ctx,
                                                    jvalue& schema,
                                                    jvalue& schema_value,
                                                    jvalue& instance,
                                                    std::string& error_msg)
    {
#if UJSON_HAVE_GMPXX
        bool valid = instance.size() <= (size_t)schema_value.mpf().get_ui();
#else
        bool valid = instance.size() <= (size_t)schema_value.num();
#endif
        if (!valid)
            error_msg = std::string("Array has more than ") + schema_value.describe() + " items.";
        return valid;
    }


    //--------------------------------------------------------------------------
    // Array instances
    //--------------------------------------------------------------------------
    bool jvocabulary_validation::validate_minItems (validation_context& ctx,
                                                    jvalue& schema,
                                                    jvalue& schema_value,
                                                    jvalue& instance,
                                                    std::string& error_msg)
    {
#if UJSON_HAVE_GMPXX
        bool valid = instance.size() >= (size_t)schema_value.mpf().get_ui();
#else
        bool valid = instance.size() >= (size_t)schema_value.num();
#endif
        if (!valid)
            error_msg = std::string("Array has less than ") + schema_value.describe() + " items.";
        return valid;
    }


    //--------------------------------------------------------------------------
    // Array instances
    //--------------------------------------------------------------------------
    bool jvocabulary_validation::validate_uniqueItems (validation_context& ctx,
                                                       jvalue& schema,
                                                       jvalue& schema_value,
                                                       jvalue& instance,
                                                       std::string& error_msg)
    {
        if (schema_value == false)
            return true;

        bool valid = true;
        if (schema_value.boolean()) {
            auto& a = instance.array ();
            for (size_t i=0; i<a.size() && valid; ++i) {
                for (size_t j=i+1; j<a.size() && valid; ++j) {
                    if (a[i] == a[j])
                        valid = false;
                }
            }
        }
        if (!valid)
            error_msg = std::string ("Array items are not unique.");
        return valid;
    }


    //--------------------------------------------------------------------------
    // Array instances
    //--------------------------------------------------------------------------
    bool jvocabulary_validation::validate_maxContains (validation_context& ctx,
                                                       jvalue& schema,
                                                       jvalue& schema_value,
                                                       jvalue& instance,
                                                       std::string& error_msg)
    {
        jvalue* annotation = nullptr;
        annotation = ctx.annotation ("contains", ctx.instance_path().str());
        if (!annotation) {
            // 6.4.4.
            //     If "contains" is not present within the same schema object,
            //     then this keyword has no effect.

            // A non-valid return with an empty error message means it wasn't evaluated at all
            error_msg = "";
            return false;
        }

        bool valid = true;
        if (annotation->is_boolean() && annotation->boolean()) {
            valid = instance.size() <= schema_value.num();
        }else{
            valid = annotation->size() <= schema_value.num();
        }

        if (!valid)
            error_msg = std::string("Array contains more than ") + schema_value.describe() + " valid items.";

        return valid;
    }


    //--------------------------------------------------------------------------
    // Array instances
    //--------------------------------------------------------------------------
    bool jvocabulary_validation::validate_minContains (validation_context& ctx,
                                                       jvalue& schema,
                                                       jvalue& schema_value,
                                                       jvalue& instance,
                                                       std::string& error_msg)
    {
        if (schema_value == 0)
            return true;

        bool valid = true;
        jvalue* annotation = nullptr;
        annotation = ctx.annotation ("contains", ctx.instance_path().str());
        if (!annotation) {
            // 6.4.5.
            //     If "contains" is not present within the same schema object,
            //     then this keyword has no effect.

            // A non-valid return with an empty error message means it wasn't evaluated at all
            error_msg = "";
            return false;
        }

        if (annotation->is_boolean() && annotation->boolean()) {
            valid = schema_value.num() <= instance.size();
        }else{
            valid = schema_value.num() <= annotation->size();
        }

        if (!valid)
            error_msg = std::string("Array contains less than ") + schema_value.describe() + " valid items.";

        return valid;
    }


    //--------------------------------------------------------------------------
    // Object instances
    //--------------------------------------------------------------------------
    bool jvocabulary_validation::validate_maxProperties (validation_context& ctx,
                                                         jvalue& schema,
                                                         jvalue& schema_value,
                                                         jvalue& instance,
                                                         std::string& error_msg)
    {
#if UJSON_HAVE_GMPXX
        size_t max_properties = (size_t) schema_value.mpf().get_ui ();
#else
        size_t max_properties = (size_t) schema_value.num ();
#endif
        bool valid = instance.size() <= max_properties;
        if (!valid)
            error_msg = std::string("Object has more than ") + std::to_string(max_properties) + " properties.";
        return valid;
    }


    //--------------------------------------------------------------------------
    // Object instances
    //--------------------------------------------------------------------------
    bool jvocabulary_validation::validate_minProperties (validation_context& ctx,
                                                         jvalue& schema,
                                                         jvalue& schema_value,
                                                         jvalue& instance,
                                                         std::string& error_msg)
    {
#if UJSON_HAVE_GMPXX
        size_t min_properties = (size_t) schema_value.mpf().get_ui ();
#else
        size_t min_properties = (size_t) schema_value.num ();
#endif
        bool valid = instance.size() >= min_properties;
        if (!valid)
            error_msg = std::string("Object has less than ") + std::to_string(min_properties) + " properties.";
        return valid;
    }


    //--------------------------------------------------------------------------
    // Object instances
    //--------------------------------------------------------------------------
    bool jvocabulary_validation::validate_required (validation_context& ctx,
                                                    jvalue& schema,
                                                    jvalue& schema_value,
                                                    jvalue& instance,
                                                    std::string& error_msg)
    {
        bool valid = true;
        for (auto& name : schema_value.array()) {
            if (instance.has(name.str()) == false) {
                valid = false;
                error_msg = std::string("Object missing property '") + name.str() + std::string("'.");
                break;
            }
        }
        return valid;
    }


    //--------------------------------------------------------------------------
    // Object instances
    //--------------------------------------------------------------------------
    bool jvocabulary_validation::validate_dependentRequired (validation_context& ctx,
                                                             jvalue& schema,
                                                             jvalue& schema_value,
                                                             jvalue& instance,
                                                             std::string& error_msg)
    {
        for (auto& member : schema_value.obj()) {
            if (instance.has(member.first) == false)
                continue;
            for (auto& name : member.second.array()) {
                if (instance.has(name.str()) == false) {
                    error_msg = std::string("Object has property '") + member.first
                        + std::string("', but missing property '") + name.str() + std::string("'.");
                    return false;
                }
            }
        }
        return true;
    }


}
