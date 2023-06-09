/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
import { assert } from "chai";

interface CustomAttribute {
  /** The class of the CustomAttribute */
  ecclass: string;
  /** An object whose properties correspond by name to the properties of this custom attribute instance. */
  properties: { [propName: string]: any; };
}

interface PropertyMetaDataProps {
  primitiveType?: number;
  structName?: string;
  extendedType?: string;
  description?: string;
  displayLabel?: string;
  minimumValue?: any;
  maximumValue?: any;
  minimumLength?: number;
  maximumLength?: number;
  readOnly?: boolean;
  kindOfQuantity?: string;
  isCustomHandled?: boolean;
  isCustomHandledOrphan?: boolean;
  minOccurs?: number;
  maxOccurs?: number;
  direction?: string;
  relationshipClass?: string;
  customAttributes?: CustomAttribute[];
}

interface EntityMetaDataProps {
  ecclass: string;
  description?: string;
  modifier?: string;
  displayLabel?: string;
  /** The  base classes from which this class derives. If more than one, the first is the super class and the others are [mixins]($docs/bis/ec/ec-mixin-class). */
  baseClasses: string[];
  /** The Custom Attributes for this class */
  customAttributes?: CustomAttribute[];
  /** An object whose properties correspond by name to the properties of this class. */
  properties: { [propName: string]: PropertyMetaDataProps; };
}

const registry = new Map<string, EntityMetaDataProps>();

export function clearRegistry() {
  registry.clear();
}

export function loadMetaData(db: any, classFullName: string): EntityMetaDataProps {
  let metaData: EntityMetaDataProps | undefined = registry.get(classFullName);
  if (metaData)
    return metaData;

  const className = classFullName.split(":");
  assert.isTrue(className.length === 2);

  const val = db.getECClassMetaData(className[0], className[1]);
  assert.isUndefined(val.error);

  metaData = JSON.parse(val.result);
  if (metaData === undefined)
    throw new Error("failed");
  registry.set(classFullName, metaData);

  // Recursive, to make sure that base classes are cached.
  if (metaData.baseClasses !== undefined && metaData.baseClasses.length > 0)
    metaData.baseClasses.forEach((baseClassName: string) => loadMetaData(db, baseClassName));

  return metaData;
}
