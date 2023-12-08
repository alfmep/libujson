/*
 * Copyright (C) 2017,2019-2023 Dan Arrhenius <dan@ultramarin.se>
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
#ifndef UJSON_HPP
#define UJSON_HPP

/**
 * Namespace for libujson.
 */
namespace ujson {}

#include <ujson/config.hpp>
#include <ujson/multimap_list.hpp>
#include <ujson/json_type_error.hpp>
#include <ujson/utils.hpp>
#include <ujson/jvalue.hpp>
#include <ujson/jpointer.hpp>
#include <ujson/jparser.hpp>
#include <ujson/invalid_schema.hpp>
#include <ujson/jschema.hpp>
#include <ujson/schema/validation_context.hpp>
#include <ujson/schema/jvocabulary.hpp>
#include <ujson/schema/jvocabulary_core.hpp>
#include <ujson/schema/jvocabulary_applicator.hpp>
#include <ujson/schema/jvocabulary_validation.hpp>
#include <ujson/schema/jvocabulary_unevaluated.hpp>

#endif
